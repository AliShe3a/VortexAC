#include "SAC.h"
#include <thread>
#include <cmath>
#include <d3dx9.h>
#include <mutex>
#include <set>
#include "VortexHooks.h" 

#pragma comment(lib, "winmm.lib")

#define PCM_SAMPLE_RATE 16000
#define PCM_CHANNELS 1
#define PCM_BITS_PER_SAMPLE 16
#define VOIP_BUFFER_SIZE 1920 

ID3DXFont* g_pVOIPFont = nullptr;

// ============================================================================
// دوال مساعدة لجلب بيانات اللعبة (روم، تيم، مود، اسم)
// ============================================================================
int She3aAC::GetCurrentGameRoomID() {
    DWORD dwRoomPortAddy = ((DWORD)(GetModuleHandleA("CShell.dll")) + 0x171D84C);
    if (dwRoomPortAddy != NULL && !IsBadReadPtr((void*)dwRoomPortAddy, sizeof(int))) {
        int roomPort = *reinterpret_cast<int*>(dwRoomPortAddy);
        if (roomPort > 0) return roomPort;
    }

    DWORD dwRoomInfo = ((DWORD)(GetModuleHandleA("CShell.dll")) + 0x1668228);
    if (dwRoomInfo != NULL) {
        auto RoomManagerAddy = *reinterpret_cast<uintptr_t*>(dwRoomInfo);
        CRoomManager* room = reinterpret_cast<CRoomManager*>(RoomManagerAddy);
        if (room && room->RoomInfo) return room->RoomInfo->RoomID;
    }
    return 0;
}

int She3aAC::GetCurrentTeamID() {
    int res = 0;
    if (!GameEngine || !GameEngine->CLTClientShell) return 0;
    auto pLocal = GameEngine->CLTClientShell->GetLocalPlayer();
    if (!pLocal) return 0;
    else res = pLocal->bTeam;
    return res;
}

bool She3aAC::IsTeamMode() {
    GAME_MODES eMode = (GAME_MODES)(-1);
    DWORD dwRoomInfo = ((DWORD)(GetModuleHandleA("CShell.dll")) + 0x1668228);
    if (dwRoomInfo != NULL) {
        auto RoomManagerAddy = *reinterpret_cast<uintptr_t*>(dwRoomInfo);
        CRoomManager* room = reinterpret_cast<CRoomManager*>(RoomManagerAddy);
        if (room && room->RoomInfo)
        {
            eMode = room->RoomInfo->GameMode;
            if (eMode == HeroModeX || eMode == ZombieKnightMode || eMode == ZombieVsGhost || eMode == ZombieMode || eMode == HeroMode || eMode == MutantChallenge
                || eMode == ZA || eMode == ZA2 || eMode == ZA3
                || eMode == FreeForAll || eMode == CopsAndRobbers)
                return false;
        }
    }
    return true;
}

std::string She3aAC::GetCurrentIGN() {
    if (She3aAC::Instance && She3aAC::Instance->cPlayer)
        return She3aAC::Instance->cPlayer->UserIGN;
    return "Player";
}

// ============================================================================
// [1] تسجيل وإرسال الصوت (Capture Worker)
// ============================================================================
unsigned __stdcall She3aAC::VoiceCaptureWorker(void* lpParam) {
    She3aAC* ac = (She3aAC*)lpParam;
    timeBeginPeriod(1);

    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, PCM_CHANNELS, PCM_SAMPLE_RATE,
                         PCM_SAMPLE_RATE * PCM_CHANNELS * (PCM_BITS_PER_SAMPLE / 8),
                         PCM_CHANNELS * (PCM_BITS_PER_SAMPLE / 8), PCM_BITS_PER_SAMPLE, 0 };

    HWAVEIN hWaveIn;
    if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 0, 0, WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR) return 0;

    const int NUM_CAP_BUFS = 3;
    WAVEHDR capHeaders[NUM_CAP_BUFS];
    char capBuffers[NUM_CAP_BUFS][VOIP_BUFFER_SIZE];

    for (int i = 0; i < NUM_CAP_BUFS; i++) {
        ZeroMemory(&capHeaders[i], sizeof(WAVEHDR));
        capHeaders[i].lpData = capBuffers[i];
        capHeaders[i].dwBufferLength = VOIP_BUFFER_SIZE;
        waveInPrepareHeader(hWaveIn, &capHeaders[i], sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &capHeaders[i], sizeof(WAVEHDR));
    }

    DWORD lastSilentPacket = 0;
    int silentPacketsSent = 0;
    const int NOISE_THRESHOLD = 350; 
    int curCapBuf = 0;
    BYTE sendBuffer[VOIP_BUFFER_SIZE + sizeof(UDP_FRAME_HEADER) + 128];

    waveInStart(hWaveIn);

    while (ac->VOIPData.isConnected) {
        while (!(capHeaders[curCapBuf].dwFlags & WHDR_DONE)) {
            if (!ac->VOIPData.isConnected) break;
            Sleep(1);
        }
        if (!ac->VOIPData.isConnected) break;

        if (ac->GetCurrentGameRoomID() > 0) {
            if (!ac->VOIPData.isMicMuted) {
                int16_t* pcmData = (int16_t*)capBuffers[curCapBuf];
                int numSamples = capHeaders[curCapBuf].dwBytesRecorded / 2;
                int maxAmplitude = 0;

                for (int i = 0; i < numSamples; i++) {
                    int32_t amplified = (int32_t)(pcmData[i] * ac->VOIPData.micVolume);
                    pcmData[i] = (int16_t)(amplified > 32767 ? 32767 : (amplified < -32768 ? -32768 : amplified));
                    if (abs(pcmData[i]) > maxAmplitude) maxAmplitude = abs(pcmData[i]);
                }

                if (maxAmplitude > NOISE_THRESHOLD) {
                    UDP_FRAME_HEADER* hdr = (UDP_FRAME_HEADER*)sendBuffer;
                    hdr->Type = UDP_TYPE_VOICE;
                    hdr->USN = ac->cPlayer->UserUSN + 21;
                    hdr->RoomID = ac->GetCurrentGameRoomID();
                    hdr->Channel = ac->VOIPData.currentChannel;
                    hdr->TeamID = ac->GetCurrentTeamID();
                    hdr->IsTalking = true;
                    strncpy(hdr->IGN, ac->GetCurrentIGN().c_str(), 15);
                    hdr->PayloadLen = capHeaders[curCapBuf].dwBytesRecorded;

                    memcpy(sendBuffer + sizeof(UDP_FRAME_HEADER), capBuffers[curCapBuf], hdr->PayloadLen);
                    sendto(ac->VOIPData.udpVoiceSocket, (const char*)sendBuffer, sizeof(UDP_FRAME_HEADER) + hdr->PayloadLen, 0, (SOCKADDR*)&ac->VOIPData.udpServerAddr, sizeof(ac->VOIPData.udpServerAddr));

                    lastSilentPacket = GetTickCount(); silentPacketsSent = 0;
                }
                else if (GetTickCount() - lastSilentPacket > 300 && silentPacketsSent < 3) {
                    UDP_FRAME_HEADER hdr = { UDP_TYPE_VOICE, (unsigned long)(ac->cPlayer->UserUSN + 21), (unsigned long)ac->GetCurrentGameRoomID() };
                    hdr.IsTalking = false; hdr.PayloadLen = 0;
                    sendto(ac->VOIPData.udpVoiceSocket, (const char*)&hdr, sizeof(hdr), 0, (SOCKADDR*)&ac->VOIPData.udpServerAddr, sizeof(ac->VOIPData.udpServerAddr));
                    lastSilentPacket = GetTickCount(); silentPacketsSent++;
                }
            }
        }

        waveInUnprepareHeader(hWaveIn, &capHeaders[curCapBuf], sizeof(WAVEHDR));
        ZeroMemory(&capHeaders[curCapBuf], sizeof(WAVEHDR));
        capHeaders[curCapBuf].lpData = capBuffers[curCapBuf];
        capHeaders[curCapBuf].dwBufferLength = VOIP_BUFFER_SIZE;
        waveInPrepareHeader(hWaveIn, &capHeaders[curCapBuf], sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &capHeaders[curCapBuf], sizeof(WAVEHDR));
        curCapBuf = (curCapBuf + 1) % NUM_CAP_BUFS;
    }

    waveInReset(hWaveIn);
    waveInClose(hWaveIn);
    timeEndPeriod(1);
    return 0;
}

// ============================================================================
// [2] استلام وتشغيل الصوت (Playback Worker)
// ============================================================================
struct PlayerAudioChannel {
    HWAVEOUT hWaveOut;
    WAVEHDR playHeaders[5];
    char playBuffers[5][VOIP_BUFFER_SIZE + 500];
    int curPlayBuf;
    DWORD lastPacketTime;
};

unsigned __stdcall She3aAC::VoicePlaybackWorker(void* lpParam) {
    She3aAC* ac = (She3aAC*)lpParam;
    timeBeginPeriod(1);
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, PCM_CHANNELS, PCM_SAMPLE_RATE, PCM_SAMPLE_RATE * 2, 2, 16, 0 };
    std::map<unsigned long, PlayerAudioChannel*> audioChannels;

    char recvBuf[65000];
    while (ac->VOIPData.isConnected) {
        int bytesRead = recvfrom(ac->VOIPData.udpVoiceSocket, recvBuf, sizeof(recvBuf), 0, NULL, NULL);
        if (bytesRead < sizeof(UDP_FRAME_HEADER)) continue;

        UDP_FRAME_HEADER* hdr = (UDP_FRAME_HEADER*)recvBuf;
        if (hdr->Type != UDP_TYPE_VOICE) continue;

        ac->VOIPData.playersMutex.lock();
        VOIPPlayer& p = ac->VOIPData.ActivePlayers[hdr->USN];
        p.USN = hdr->USN; p.IGN = hdr->IGN; p.isTalking = hdr->IsTalking; p.lastPacketTime = GetTickCount();
        bool isMuted = ac->VOIPData.MutedPlayers.count(hdr->USN) > 0;
        ac->VOIPData.playersMutex.unlock();

        if (hdr->IsTalking && !isMuted && !ac->VOIPData.isSpeakerMuted && hdr->PayloadLen > 0) {
            if (audioChannels.find(hdr->USN) == audioChannels.end()) {
                PlayerAudioChannel* newCh = new PlayerAudioChannel();
                newCh->curPlayBuf = 0; newCh->lastPacketTime = GetTickCount();
                for (int i = 0; i < 5; i++) ZeroMemory(&newCh->playHeaders[i], sizeof(WAVEHDR));
                waveOutOpen(&newCh->hWaveOut, WAVE_MAPPER, &wfx, 0, 0, WAVE_FORMAT_DIRECT);
                audioChannels[hdr->USN] = newCh;
            }

            PlayerAudioChannel* ch = audioChannels[hdr->USN];
            ch->lastPacketTime = GetTickCount();
            int b = ch->curPlayBuf;
            if (ch->playHeaders[b].dwFlags & WHDR_PREPARED) {
                while (!(ch->playHeaders[b].dwFlags & WHDR_DONE) && ac->VOIPData.isConnected) Sleep(1);
                waveOutUnprepareHeader(ch->hWaveOut, &ch->playHeaders[b], sizeof(WAVEHDR));
            }

            int copyLen = (hdr->PayloadLen > sizeof(ch->playBuffers[b])) ? sizeof(ch->playBuffers[b]) : hdr->PayloadLen;
            memcpy(ch->playBuffers[b], recvBuf + sizeof(UDP_FRAME_HEADER), copyLen);
            ZeroMemory(&ch->playHeaders[b], sizeof(WAVEHDR));
            ch->playHeaders[b].lpData = ch->playBuffers[b]; ch->playHeaders[b].dwBufferLength = copyLen;
            waveOutPrepareHeader(ch->hWaveOut, &ch->playHeaders[b], sizeof(WAVEHDR));
            waveOutWrite(ch->hWaveOut, &ch->playHeaders[b], sizeof(WAVEHDR));
            ch->curPlayBuf = (b + 1) % 5;
        }

        for (auto it = audioChannels.begin(); it != audioChannels.end(); ) {
            if (GetTickCount() - it->second->lastPacketTime > 3000) {
                waveOutReset(it->second->hWaveOut);
                waveOutClose(it->second->hWaveOut);
                delete it->second; it = audioChannels.erase(it);
            }
            else ++it;
        }
    }
    for (auto& pair : audioChannels) { waveOutReset(pair.second->hWaveOut); waveOutClose(pair.second->hWaveOut); delete pair.second; }
    return 0;
}
void She3aAC::ToggleVOIPConnection() {
    if (VOIPData.isConnected) {
        VOIPData.isConnected = false;
        if (VOIPData.udpVoiceSocket != INVALID_SOCKET) {
            closesocket(VOIPData.udpVoiceSocket);
            VOIPData.udpVoiceSocket = INVALID_SOCKET;
        }
        if (VOIPData.hCaptureThread) {
            WaitForSingleObject(VOIPData.hCaptureThread, 1000);
            CloseHandle(VOIPData.hCaptureThread);
            VOIPData.hCaptureThread = NULL;
        }
        if (VOIPData.hPlaybackThread) {
            WaitForSingleObject(VOIPData.hPlaybackThread, 1000);
            CloseHandle(VOIPData.hPlaybackThread);
            VOIPData.hPlaybackThread = NULL;
        }

        VOIPData.playersMutex.lock();
        VOIPData.ActivePlayers.clear();
        VOIPData.playersMutex.unlock();
    }
    else {
        VOIPData.udpVoiceSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (VOIPData.udpVoiceSocket == INVALID_SOCKET) return;

        memset(&VOIPData.udpServerAddr, 0, sizeof(VOIPData.udpServerAddr));
        VOIPData.udpServerAddr.sin_family = AF_INET;
        VOIPData.udpServerAddr.sin_port = htons(1889);
        VOIPData.udpServerAddr.sin_addr.s_addr = inet_addr(GetCFServerIP().c_str());

        VOIPData.isConnected = true;
        VOIPData.hCaptureThread = (HANDLE)_beginthreadex(NULL, 0, VoiceCaptureWorker, this, 0, NULL);
        VOIPData.hPlaybackThread = (HANDLE)_beginthreadex(NULL, 0, VoicePlaybackWorker, this, 0, NULL);
    }
}

#define D3DFVF_VOIPVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct VOIPVERTEX { float x, y, z, rhw; D3DCOLOR color; };

void DrawRoundedRect(IDirect3DDevice9* pDevice, float x, float y, float width, float height, float radius, D3DCOLOR color) {
    if (!pDevice) return;
    const int resolution = 8;
    VOIPVERTEX vertices[50];
    int index = 0;

    vertices[index++] = { x + width / 2.0f, y + height / 2.0f, 0.0f, 1.0f, color };
    for (int i = 0; i <= resolution; i++) { float angle = 1.570796f + (1.570796f * i / resolution); vertices[index++] = { x + radius + cosf(angle) * radius, y + radius - sinf(angle) * radius, 0.0f, 1.0f, color }; }
    for (int i = 0; i <= resolution; i++) { float angle = 3.141592f + (1.570796f * i / resolution); vertices[index++] = { x + radius + cosf(angle) * radius, y + height - radius - sinf(angle) * radius, 0.0f, 1.0f, color }; }
    for (int i = 0; i <= resolution; i++) { float angle = 4.712388f + (1.570796f * i / resolution); vertices[index++] = { x + width - radius + cosf(angle) * radius, y + height - radius - sinf(angle) * radius, 0.0f, 1.0f, color }; }
    for (int i = 0; i <= resolution; i++) { float angle = 0.0f + (1.570796f * i / resolution); vertices[index++] = { x + width - radius + cosf(angle) * radius, y + radius - sinf(angle) * radius, 0.0f, 1.0f, color }; }
    vertices[index] = vertices[1];

    pDevice->SetFVF(D3DFVF_VOIPVERTEX);
    if (GameHooks.oDrawPrimitiveUP) GameHooks.oDrawPrimitiveUP(pDevice, D3DPT_TRIANGLEFAN, index - 1, vertices, sizeof(VOIPVERTEX));
    else pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, index - 1, vertices, sizeof(VOIPVERTEX));
}

void DrawSimpleRect(IDirect3DDevice9* pDevice, float x, float y, float width, float height, D3DCOLOR color) {
    VOIPVERTEX vertices[4] = { { x, y + height, 0.0f, 1.0f, color }, { x, y, 0.0f, 1.0f, color }, { x + width, y + height, 0.0f, 1.0f, color }, { x + width, y, 0.0f, 1.0f, color } };
    pDevice->SetFVF(D3DFVF_VOIPVERTEX);
    if (GameHooks.oDrawPrimitiveUP) GameHooks.oDrawPrimitiveUP(pDevice, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VOIPVERTEX));
    else pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VOIPVERTEX));
}

void DrawMicIcon(IDirect3DDevice9* pDevice, float cx, float cy, D3DCOLOR color) {
    DrawRoundedRect(pDevice, cx - 2, cy - 6, 4, 10, 2, color);
    DrawSimpleRect(pDevice, cx - 5, cy - 1, 2, 5, color);
    DrawSimpleRect(pDevice, cx + 3, cy - 1, 2, 5, color);
    DrawSimpleRect(pDevice, cx - 5, cy + 4, 10, 2, color);
    DrawSimpleRect(pDevice, cx - 1, cy + 6, 2, 4, color);
    DrawSimpleRect(pDevice, cx - 4, cy + 10, 8, 2, color);
}

void DrawTextLeft(IDirect3DDevice9* pDevice, int x, int y, int w, int h, const char* text, D3DCOLOR color) {
    if (!g_pVOIPFont) D3DXCreateFontA(pDevice, 13, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pVOIPFont);
    if (g_pVOIPFont) {
        RECT rect = { x, y, x + w, y + h };
        g_pVOIPFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE, color);
    }
}

void DrawTextCentered(IDirect3DDevice9* pDevice, int x, int y, int w, int h, const char* text, D3DCOLOR color) {
    if (!g_pVOIPFont) D3DXCreateFontA(pDevice, 13, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pVOIPFont);
    if (g_pVOIPFont) {
        RECT rect = { x, y, x + w, y + h };
        g_pVOIPFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE, color);
    }
}

// =========================================================================================
// رسم المنيو (UI) والـ Overlay الذكي
// =========================================================================================
void She3aAC::RenderVOIPUI(IDirect3DDevice9* pDevice) {
    __try {
        if (!pDevice) return;

        D3DVIEWPORT9 vp; pDevice->GetViewport(&vp);
        DWORD currentTime = GetTickCount();
        if (VOIPData.lastFrameTime == 0) VOIPData.lastFrameTime = currentTime;
        float dt = (float)(currentTime - VOIPData.lastFrameTime);
        VOIPData.lastFrameTime = currentTime;

        float animSpeed = 0.008f;
        if (VOIPData.isUIVisible) { VOIPData.animProgress += dt * animSpeed; if (VOIPData.animProgress > 1.0f) VOIPData.animProgress = 1.0f; }
        else { VOIPData.animProgress -= dt * animSpeed; if (VOIPData.animProgress < 0.0f) VOIPData.animProgress = 0.0f; }

        VOIPData.playersMutex.lock();
        for (auto it = VOIPData.ActivePlayers.begin(); it != VOIPData.ActivePlayers.end(); ) {
            if (currentTime - it->second.lastPacketTime > 2000) it = VOIPData.ActivePlayers.erase(it);
            else ++it;
        }
        VOIPData.playersMutex.unlock();

        IDirect3DVertexShader9* pOldVS = nullptr; IDirect3DPixelShader9* pOldPS = nullptr; IDirect3DBaseTexture9* pOldTex = nullptr;
        DWORD oldFVF, oldZEnable, oldAlphaBlend, oldSrcBlend, oldDestBlend, oldCullMode, oldAlphaTest;
        pDevice->GetVertexShader(&pOldVS); pDevice->GetPixelShader(&pOldPS); pDevice->GetTexture(0, &pOldTex);
        pDevice->GetFVF(&oldFVF); pDevice->GetRenderState(D3DRS_ZENABLE, &oldZEnable);
        pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend); pDevice->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
        pDevice->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend); pDevice->GetRenderState(D3DRS_CULLMODE, &oldCullMode);
        pDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &oldAlphaTest);

        pDevice->SetVertexShader(nullptr); pDevice->SetPixelShader(nullptr); pDevice->SetTexture(0, nullptr);
        pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE); pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

        VOIPData.iconW = 18; VOIPData.iconH = 40;
        VOIPData.iconX = vp.Width - VOIPData.iconW;
        VOIPData.iconY = (int)(vp.Height * 0.25f);

        D3DCOLOR iconBgColor = VOIPData.isConnected ? D3DCOLOR_ARGB(200, 0, 150, 255) : D3DCOLOR_ARGB(200, 30, 30, 35);
        DrawRoundedRect(pDevice, VOIPData.iconX, VOIPData.iconY, VOIPData.iconW + 10, VOIPData.iconH, 6, iconBgColor);

        float micCenterX = VOIPData.iconX + (VOIPData.iconW / 2.0f);
        float micCenterY = VOIPData.iconY + (VOIPData.iconH / 2.0f) - 3;
        D3DCOLOR micIconColor = VOIPData.isMicMuted ? D3DCOLOR_ARGB(255, 230, 60, 60) : D3DCOLOR_ARGB(255, 240, 240, 240);
        DrawMicIcon(pDevice, micCenterX, micCenterY, micIconColor);

        if (VOIPData.isConnected) {
            int overlayX = 20;
            int overlayY = vp.Height / 3;
            int drawnCount = 0;

            VOIPData.playersMutex.lock();
            for (const auto& pair : VOIPData.ActivePlayers) {
                bool isMuted = (VOIPData.MutedPlayers.count(pair.first) > 0);

                if (pair.second.isTalking && !isMuted) {
                    if (drawnCount >= 2) break;

                    int nameWidth = 120;
                    DrawRoundedRect(pDevice, overlayX, overlayY, nameWidth, 24, 12, D3DCOLOR_ARGB(150, 15, 15, 20));
                    DrawRoundedRect(pDevice, overlayX + 8, overlayY + 8, 8, 8, 4, D3DCOLOR_ARGB(255, 46, 204, 113));
                    DrawTextLeft(pDevice, overlayX + 22, overlayY + 2, nameWidth - 25, 20, pair.second.IGN.c_str(), D3DCOLOR_ARGB(255, 240, 240, 240));

                    overlayY += 30;
                    drawnCount++;
                }
            }
            VOIPData.playersMutex.unlock();
        }

        if (VOIPData.animProgress > 0.0f) {
            int targetX = vp.Width - VOIPData.iconW - 210 - 5;
            VOIPData.uiWidth = 210;
            VOIPData.uiHeight = 250;
            VOIPData.uiY = VOIPData.iconY - 20;
            VOIPData.uiX = (int)(vp.Width + 10 - (VOIPData.animProgress * (vp.Width + 10 - targetX)));

            int alpha = (int)(230 * VOIPData.animProgress);
            int textAlpha = (int)(255 * VOIPData.animProgress);

            DrawRoundedRect(pDevice, (float)VOIPData.uiX, (float)VOIPData.uiY, (float)VOIPData.uiWidth, (float)VOIPData.uiHeight, 10.0f, D3DCOLOR_ARGB(alpha, 15, 15, 18));
            DrawTextCentered(pDevice, VOIPData.uiX, VOIPData.uiY + 8, VOIPData.uiWidth, 20, "VORTEX COMMS", D3DCOLOR_ARGB(textAlpha, 220, 220, 220));
            DrawRoundedRect(pDevice, VOIPData.uiX + 20, VOIPData.uiY + 28, VOIPData.uiWidth - 40, 2, 1, D3DCOLOR_ARGB(alpha, 0, 150, 255));

            int pad = 10;
            int currentY = VOIPData.uiY + 40;

            VOIPData.btnChannelY = currentY;
            VOIPData.btnChannelW = (VOIPData.uiWidth - (pad * 3)) / 2;
            VOIPData.btnGlobalX = VOIPData.uiX + pad;
            VOIPData.btnTeamX = VOIPData.uiX + pad * 2 + VOIPData.btnChannelW;
            VOIPData.btnChannelH = 25;

            bool teamAllowed = IsTeamMode();
            D3DCOLOR globColor = (VOIPData.currentChannel == CH_GLOBAL) ? D3DCOLOR_ARGB(alpha, 0, 120, 210) : D3DCOLOR_ARGB(alpha, 30, 30, 35);
            D3DCOLOR teamColor = !teamAllowed ? D3DCOLOR_ARGB(alpha, 20, 20, 20) : ((VOIPData.currentChannel == CH_TEAM) ? D3DCOLOR_ARGB(alpha, 0, 120, 210) : D3DCOLOR_ARGB(alpha, 30, 30, 35));
            D3DCOLOR teamTxtColor = !teamAllowed ? D3DCOLOR_ARGB(textAlpha, 100, 100, 100) : D3DCOLOR_ARGB(textAlpha, 220, 220, 220);

            DrawRoundedRect(pDevice, VOIPData.btnGlobalX, VOIPData.btnChannelY, VOIPData.btnChannelW, VOIPData.btnChannelH, 4, globColor);
            DrawTextCentered(pDevice, VOIPData.btnGlobalX, VOIPData.btnChannelY, VOIPData.btnChannelW, VOIPData.btnChannelH, "GLOBAL", D3DCOLOR_ARGB(textAlpha, 220, 220, 220));

            DrawRoundedRect(pDevice, VOIPData.btnTeamX, VOIPData.btnChannelY, VOIPData.btnChannelW, VOIPData.btnChannelH, 4, teamColor);
            DrawTextCentered(pDevice, VOIPData.btnTeamX, VOIPData.btnChannelY, VOIPData.btnChannelW, VOIPData.btnChannelH, "TEAM", teamTxtColor);

            currentY += VOIPData.btnChannelH + 10;
            VOIPData.btnToolsY = currentY;
            VOIPData.btnToolsW = (VOIPData.uiWidth - (pad * 4)) / 3;
            VOIPData.btnToolsH = 25;

            VOIPData.btnMicX = VOIPData.uiX + pad;
            VOIPData.btnSpkX = VOIPData.btnMicX + VOIPData.btnToolsW + pad;
            VOIPData.btnConnX = VOIPData.btnSpkX + VOIPData.btnToolsW + pad;

            D3DCOLOR micC = VOIPData.isMicMuted ? D3DCOLOR_ARGB(alpha, 200, 50, 50) : D3DCOLOR_ARGB(alpha, 40, 180, 80);
            D3DCOLOR spkC = VOIPData.isSpeakerMuted ? D3DCOLOR_ARGB(alpha, 200, 50, 50) : D3DCOLOR_ARGB(alpha, 40, 180, 80);
            D3DCOLOR connC = VOIPData.isConnected ? D3DCOLOR_ARGB(alpha, 0, 120, 210) : D3DCOLOR_ARGB(alpha, 80, 80, 80);

            DrawRoundedRect(pDevice, VOIPData.btnMicX, VOIPData.btnToolsY, VOIPData.btnToolsW, VOIPData.btnToolsH, 4, micC);
            DrawTextCentered(pDevice, VOIPData.btnMicX, VOIPData.btnToolsY, VOIPData.btnToolsW, VOIPData.btnToolsH, "MIC", D3DCOLOR_ARGB(textAlpha, 255, 255, 255));

            DrawRoundedRect(pDevice, VOIPData.btnSpkX, VOIPData.btnToolsY, VOIPData.btnToolsW, VOIPData.btnToolsH, 4, spkC);
            DrawTextCentered(pDevice, VOIPData.btnSpkX, VOIPData.btnToolsY, VOIPData.btnToolsW, VOIPData.btnToolsH, "SPK", D3DCOLOR_ARGB(textAlpha, 255, 255, 255));

            DrawRoundedRect(pDevice, VOIPData.btnConnX, VOIPData.btnToolsY, VOIPData.btnToolsW, VOIPData.btnToolsH, 4, connC);
            DrawTextCentered(pDevice, VOIPData.btnConnX, VOIPData.btnToolsY, VOIPData.btnToolsW, VOIPData.btnToolsH, VOIPData.isConnected ? "DC" : "JOIN", D3DCOLOR_ARGB(textAlpha, 255, 255, 255));

            currentY += VOIPData.btnToolsH + 15;

            VOIPData.sliderX = VOIPData.uiX + pad;
            VOIPData.sliderY = currentY + 10;
            VOIPData.sliderW = VOIPData.uiWidth - (pad * 2);
            VOIPData.sliderH = 6;

            DrawTextLeft(pDevice, VOIPData.sliderX, currentY - 10, VOIPData.sliderW, 15, "Mic Volume", D3DCOLOR_ARGB(textAlpha, 180, 180, 180));

            DrawRoundedRect(pDevice, VOIPData.sliderX, VOIPData.sliderY, VOIPData.sliderW, VOIPData.sliderH, 3, D3DCOLOR_ARGB(alpha, 40, 40, 45));

            float fillWidth = VOIPData.sliderW * (VOIPData.micVolume / 2.0f);
            DrawRoundedRect(pDevice, VOIPData.sliderX, VOIPData.sliderY, fillWidth, VOIPData.sliderH, 3, D3DCOLOR_ARGB(alpha, 46, 204, 113));

            DrawRoundedRect(pDevice, VOIPData.sliderX + fillWidth - 5, VOIPData.sliderY - 4, 10, 14, 5, D3DCOLOR_ARGB(alpha, 255, 255, 255));

            currentY += 25;
            DrawRoundedRect(pDevice, VOIPData.uiX + pad, currentY, VOIPData.uiWidth - (pad * 2), 1, 0, D3DCOLOR_ARGB(alpha, 50, 50, 50));
            currentY += 5;

            VOIPData.playersMutex.lock();
            bool isPlayersEmpty = VOIPData.ActivePlayers.empty();
            VOIPData.playersMutex.unlock();

            if (isPlayersEmpty || !VOIPData.isConnected) {
                DrawTextCentered(pDevice, VOIPData.uiX, currentY, VOIPData.uiWidth, 30, "No active players", D3DCOLOR_ARGB(textAlpha, 150, 150, 150));
            }
            else {
                int maxDisplay = 4;
                int count = 0;

                VOIPData.playersMutex.lock();
                for (const auto& pair : VOIPData.ActivePlayers) {
                    if (count >= maxDisplay) break;

                    bool isMuted = (VOIPData.MutedPlayers.count(pair.first) > 0);

                    D3DCOLOR dotColor = pair.second.isTalking ? (isMuted ? D3DCOLOR_ARGB(textAlpha, 200, 50, 50) : D3DCOLOR_ARGB(textAlpha, 46, 204, 113)) : D3DCOLOR_ARGB(textAlpha, 100, 100, 100);
                    DrawRoundedRect(pDevice, VOIPData.uiX + pad + 5, currentY + 6, 6, 6, 3, dotColor);

                    D3DCOLOR nameColor = isMuted ? D3DCOLOR_ARGB(textAlpha, 150, 150, 150) : D3DCOLOR_ARGB(textAlpha, 220, 220, 220);
                    DrawTextLeft(pDevice, VOIPData.uiX + pad + 20, currentY, VOIPData.uiWidth - 60, 20, pair.second.IGN.c_str(), nameColor);

                    D3DCOLOR btnColor = isMuted ? D3DCOLOR_ARGB(alpha, 200, 50, 50) : D3DCOLOR_ARGB(alpha, 50, 50, 50);
                    int btnX = VOIPData.uiX + VOIPData.uiWidth - 35;
                    DrawRoundedRect(pDevice, btnX, currentY + 2, 20, 16, 3, btnColor);
                    DrawTextCentered(pDevice, btnX, currentY + 2, 20, 16, isMuted ? "U" : "M", D3DCOLOR_ARGB(textAlpha, 255, 255, 255));

                    currentY += 20;
                    count++;
                }
                VOIPData.playersMutex.unlock();
            }
        }

        pDevice->SetVertexShader(pOldVS); if (pOldVS) pOldVS->Release();
        pDevice->SetPixelShader(pOldPS); if (pOldPS) pOldPS->Release();
        pDevice->SetTexture(0, pOldTex); if (pOldTex) pOldTex->Release();
        pDevice->SetFVF(oldFVF); pDevice->SetRenderState(D3DRS_ZENABLE, oldZEnable);
        pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend); pDevice->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
        pDevice->SetRenderState(D3DRS_DESTBLEND, oldDestBlend); pDevice->SetRenderState(D3DRS_CULLMODE, oldCullMode);
        pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, oldAlphaTest);

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

void She3aAC::HandleVOIPClick(int mouseX, int mouseY) {
    if (mouseX >= VOIPData.iconX && mouseX <= VOIPData.iconX + VOIPData.iconW &&
        mouseY >= VOIPData.iconY && mouseY <= VOIPData.iconY + VOIPData.iconH) {
        VOIPData.isUIVisible = !VOIPData.isUIVisible;
        return;
    }

    if (VOIPData.animProgress > 0.5f) {
        if (mouseY >= VOIPData.sliderY - 5 && mouseY <= VOIPData.sliderY + VOIPData.sliderH + 5) {
            if (mouseX >= VOIPData.sliderX && mouseX <= VOIPData.sliderX + VOIPData.sliderW) {
                float percent = (float)(mouseX - VOIPData.sliderX) / VOIPData.sliderW;
                VOIPData.micVolume = percent * 2.0f;
                return;
            }
        }

        if (mouseY >= VOIPData.btnChannelY && mouseY <= VOIPData.btnChannelY + VOIPData.btnChannelH) {
            if (mouseX >= VOIPData.btnGlobalX && mouseX <= VOIPData.btnGlobalX + VOIPData.btnChannelW) {
                VOIPData.currentChannel = CH_GLOBAL;
            }
            else if (mouseX >= VOIPData.btnTeamX && mouseX <= VOIPData.btnTeamX + VOIPData.btnChannelW) {
                if (IsTeamMode()) VOIPData.currentChannel = CH_TEAM;
            }
        }
        else if (mouseY >= VOIPData.btnToolsY && mouseY <= VOIPData.btnToolsY + VOIPData.btnToolsH) {
            if (mouseX >= VOIPData.btnMicX && mouseX <= VOIPData.btnMicX + VOIPData.btnToolsW) {
                VOIPData.isMicMuted = !VOIPData.isMicMuted;
            }
            else if (mouseX >= VOIPData.btnSpkX && mouseX <= VOIPData.btnSpkX + VOIPData.btnToolsW) {
                VOIPData.isSpeakerMuted = !VOIPData.isSpeakerMuted;
                if (VOIPData.isSpeakerMuted) VOIPData.isMicMuted = true;
            }
            else if (mouseX >= VOIPData.btnConnX && mouseX <= VOIPData.btnConnX + VOIPData.btnToolsW) {
                ToggleVOIPConnection();
            }
        }

        int listY = VOIPData.uiY + 145;
        int count = 0;

        VOIPData.playersMutex.lock();
        for (const auto& pair : VOIPData.ActivePlayers) {
            if (count >= 4) break;

            int btnX = VOIPData.uiX + VOIPData.uiWidth - 35;
            int btnY = listY + 2;

            // لو اليوزر ضغط على مساحة الزرار
            if (mouseX >= btnX && mouseX <= btnX + 20 && mouseY >= btnY && mouseY <= btnY + 16) {
                if (VOIPData.MutedPlayers.count(pair.first)) {
                    VOIPData.MutedPlayers.erase(pair.first); 
                }
                else {
                    VOIPData.MutedPlayers.insert(pair.first); 
                }
                break;
            }
            listY += 20;
            count++;
        }
        VOIPData.playersMutex.unlock();
    }
}