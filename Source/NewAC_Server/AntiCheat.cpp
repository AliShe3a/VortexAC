#include "AntiCheat.h"
#include "CRC.h"
#include <thread>
#include <wincrypt.h>
#include <sstream>
#include <iomanip>
#include "defines.h"

#pragma comment(lib, "advapi32.lib")
/**
 * @file AntiCheat.cpp
 * @brief تنفيذ عمليات النيتورك والتحكم في اللاعبين (Implementation)
 */

struct DiscordLogItem {
    std::string channelId;
    std::string payload;
    std::string tokenOverride; 
    bool isWebhook;
};

std::queue<DiscordLogItem> g_DiscordQueue;
std::mutex g_LogMutex;
std::condition_variable g_LogCV;
bool g_StopLogWorker = false;

const size_t MAX_ALLOWED_CHUNK = 8192 + 512;
const size_t MAX_STREAM_BUFFER = 2 * 1024 * 1024;
const int MAX_PACKETS_PER_SECOND = 1000;
const size_t MAX_SRV_CHUNK_SIZE = 8192;

const BYTE EncryptionKey[] = { 0x87, 0x1A, 0x3C, 0x47, 0x9F, 0xE5, 0x56, 0x52, 0xEB, 0xCF, 0x71, 0x20, 0xB3, 0xAE, 0x92, 0xDE, 0x2B, 0x4E, 0xBD, 0x20, 0xC6, 0x5C, 0x13, 0xE2, 0x74, 0x0F, 0xAF, 0x6A, 0x0F, 0xA1, 0x6E, 0x28, 0x1D, 0x01, 0x19, 0x58, 0x57, 0x55, 0x80, 0xDC, 0xC8, 0x8E, 0xC6, 0xA9, 0xD5, 0x88, 0x84, 0x0C, 0x29, 0xDD, 0x25, 0x1B, 0xC4, 0xA8, 0xE7, 0x2B, 0x06, 0x59, 0xD1, 0x90, 0x35, 0xEA, 0x43, 0x4A, 0x61, 0x48, 0x03, 0xB1, 0x95, 0x3A, 0x86, 0xD5, 0xCC, 0xDF, 0x56, 0x37, 0x94, 0x68, 0x0C, 0xEB, 0x89, 0xA3, 0x41, 0x31, 0x7B, 0x52, 0x63, 0x74, 0x18, 0xEC, 0x51, 0xFD, 0x82, 0xEF, 0xA6, 0xDD, 0x4B, 0x7C, 0x48, 0x8D, 0x85, 0xD1, 0x76, 0xED, 0x56, 0x4F, 0x4D, 0xE3, 0x51, 0xB0, 0x95, 0x9D, 0x4C, 0xE5, 0x06, 0x91, 0xA8, 0x11, 0xCD, 0x0E, 0xE4, 0x8D, 0x99, 0xA7, 0xE7, 0x88, 0x52, 0xDC, 0x39, 0x78, 0xEB, 0x18, 0x20, 0x7E, 0x66, 0xB6, 0x2C, 0x52, 0xDA, 0x2B, 0x05, 0x7F, 0x22, 0x67, 0x56, 0xB3, 0xB1, 0x9C, 0xE8, 0x14, 0x34, 0x39, 0xAD, 0x39, 0xA7, 0xF6, 0x38, 0xCE, 0x06, 0xCC, 0xFC, 0x70, 0xA9, 0xD7, 0xC3, 0x7D, 0x7D, 0xCC, 0x7F, 0x4F, 0x65, 0x56, 0x36, 0xA2, 0x09, 0x64, 0x6B, 0x32, 0x50, 0x34, 0xD6, 0xB5, 0xF2, 0x25, 0x44, 0xB1, 0x6A, 0xE5, 0xD5, 0x6F, 0x7E, 0x5D, 0x4C, 0x0D, 0x2E, 0x93, 0xD4, 0x4A, 0xA2, 0xD8, 0x35, 0xE3, 0x1D, 0x99, 0xAE, 0xDC, 0x03, 0x0B, 0x2F, 0x43, 0x8A, 0x06, 0x7D, 0xD0, 0xF5, 0x46, 0x7D, 0x84, 0x92, 0x12, 0x12, 0x96, 0xF5, 0x10, 0xCF, 0x07, 0x09, 0x28, 0x73, 0x74, 0x8E, 0x21, 0x8E, 0x76, 0xFE, 0xF1, 0x98, 0x58, 0x75, 0x66, 0xAE, 0x5F, 0xB6, 0x92, 0xE3, 0x77, 0xDE, 0x7B, 0x17, 0xE6, 0x16, 0x50, 0xB6, 0x90, 0xC6, 0x9E };

She3aSrv* She3aSrv::Instance = nullptr;

She3aSrv::She3aSrv() {
    ListenSocket = INVALID_SOCKET;
    db = std::make_unique<DatabaseManager>();
    discord = std::make_unique<DiscordManager>();
    webServer = std::make_unique<AnticheatWebServer>();
}

She3aSrv::~She3aSrv() {
    if (webServer) webServer->Stop();
    if (db) db->Disconnect();
    if (ListenSocket != INVALID_SOCKET) closesocket(ListenSocket);
    WSACleanup();
}

void She3aSrv::EncryptDatas(BYTE* datas, size_t len) {
    for (int i = 0; i < len; i++) {
        datas[i] ^= EncryptionKey[i % 256];
        datas[i] = ~datas[i];
        datas[i] ^= EncryptionKey[255 - i % 256];
    }
}

void She3aSrv::DecryptDatas(BYTE* datas, size_t len) {
    for (int i = 0; i < len; i++) {
        datas[i] ^= EncryptionKey[255 - i % 256];
        datas[i] = ~datas[i];
        datas[i] ^= EncryptionKey[i % 256];
    }
}

const char* She3aSrv::GetAuthorityName(PLAYER_TYPE type) {
    switch (type) {
    case GM_PLAYER:    return "GameMaster";
    case ADMIN_PLAYER: return "Adminstrator";
    case NORMAL_PLAYER:
    default:           return "Normal Player";
    }
}

std::string She3aSrv::ComputeMD5Hash(const std::string& input) {
    std::string hexDigest;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
            if (CryptHashData(hHash, reinterpret_cast<const BYTE*>(input.data()), static_cast<DWORD>(input.size()), 0)) {
                DWORD hashLen = 16;
                BYTE hash[16];
                if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
                    std::ostringstream oss;
                    oss << std::hex << std::uppercase << std::setfill('0');
                    for (DWORD i = 0; i < hashLen; ++i) {
                        oss << std::setw(2) << static_cast<int>(hash[i]);
                    }
                    hexDigest = oss.str();
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return hexDigest;
}

bool She3aSrv::InitDatabase() {
    if (db && db->Connect()) {
        this->Log(L_SUCCESS, "Database", "Database Manager Initialized and Connected.");
        return true;
    }
    this->Log(L_ERROR, "Database", "Database Manager failed to connect!");
    return false;
}

void She3aSrv::ReportError(std::string msg) {
    this->Log(L_ERROR, "System", msg + " | Code: " + std::to_string(WSAGetLastError()));
}

bool She3aSrv::Init() {
    const char* folders[] = { "./Screenshots", "./Screenshots/Auth", "./Screenshots/Heartbeat","./Screenshots/Banned", "./Files" , "./Decrypted_Files" };
    for (const char* folder : folders) {
        if (!CreateDirectoryA(folder, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
            this->Log(L_ERROR, "System", "Failed to create folder: " + std::string(folder));
            return false;
        }
    }

    LoadConfig();

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ServerIP.c_str());
    serverAddr.sin_port = htons(tServerPort);

    if (bind(ListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        this->Log(L_ERROR, "Network", "Bind WS Connection Failed!");
        closesocket(ListenSocket);
        return false;
    }

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        this->Log(L_ERROR, "Network", "Listen WS Connection Failed!");
        closesocket(ListenSocket);
        return false;
    }

    UdpListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (UdpListenSocket != INVALID_SOCKET) {
        sockaddr_in udpAddr;
        udpAddr.sin_family = AF_INET;
        udpAddr.sin_addr.s_addr = inet_addr(ServerIP.c_str());
        udpAddr.sin_port = htons(uServerPort); 

        if (bind(UdpListenSocket, (sockaddr*)&udpAddr, sizeof(udpAddr)) != SOCKET_ERROR) {
            std::thread(&She3aSrv::UdpListenLoop, this).detach();
            Log(L_SUCCESS, "Network", "Live Share UDP Server active on port 1889");
        }
        else {
            Log(L_ERROR, "Network", "Failed to bind UDP port 1889");
        }
    }


    if (!InitServices()) {
        this->Log(L_ERROR, "Service", "Failed To Init Services!");
        return false;
    }

    this->Log(L_SUCCESS, "Vortex", "Server Initialized on : " + ServerIP + ":" + std::to_string(tServerPort) + ". Ready for incoming connections.");
    return true;
}

bool She3aSrv::InitServices() {
    this->Log(L_INFO, "Vortex", "Initializing Core Services...");
    if (!db->Connect()) {
        this->Log(L_ERROR, "Database", "SQL Connection failed! Check DB settings.");
        return false;
    }
    this->Log(L_SUCCESS, "Service", "SQL Database: Connected.");

    if (!discord->Init(CustomBotToken)) {

        if(CustomBotToken == "CustomBotToken")
            this->Log(L_WARN, "Discord", "Please Setup Discord Bot Token From WebPanel.");
        else
        this->Log(L_ERROR, "Discord", "Discord Bot failed to initialize!");
    }
    else {
        this->Log(L_SUCCESS, "Service", "Discord Bot: Online (Gateway Active).");
        std::thread(&She3aSrv::DiscordLogWorker, this).detach();
        this->Log(L_SUCCESS, "Service", "Discord Log Worker: Started.");
    }

    if (!webServer->Start(WEB_SERVER_PORT, "./", ServerIP)) {
        this->Log(L_ERROR, "WebServer", "WebServer failed to start on port " + std::to_string(WEB_SERVER_PORT));
        return false;
    }
    this->Log(L_SUCCESS, "Service", "Anticheat WebServer: Active on http://" + ServerIP + ":" + std::to_string(WEB_SERVER_PORT));

    std::thread(&She3aSrv::MonitorWorker).detach();
    this->Log(L_SUCCESS, "Service", "Monitor Worker: Started.");

    this->Log(L_SUCCESS, "System", "All services are ready. Vortex Server is LIVE.");
    return true;
}

void She3aSrv::DiscordLogWorker() {
    while (!g_StopLogWorker) {
        DiscordLogItem item;
        {
            std::unique_lock<std::mutex> lock(g_LogMutex);
            g_LogCV.wait(lock, [] { return !g_DiscordQueue.empty() || g_StopLogWorker; });
            if (g_StopLogWorker && g_DiscordQueue.empty()) break;
            item = g_DiscordQueue.front();
            g_DiscordQueue.pop();
        }

        if (Instance && Instance->discord) {
            if (item.isWebhook) {
                Instance->discord->PostToWebhook(item.channelId, item.payload);
            }
            else {
                Instance->discord->SendLog(item.channelId, item.payload, item.tokenOverride);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void EnqueueLog(const std::string& target, const std::string& payload, const std::string& tokenOverride = "", bool isWebhook = false) {
    {
        std::lock_guard<std::mutex> lock(g_LogMutex);
        g_DiscordQueue.push({ target, payload, tokenOverride, isWebhook });
    }
    g_LogCV.notify_one();
}

void She3aSrv::LogAuthToDiscord(std::shared_ptr<PlayerInfo> player, const std::string& screenshotUrl) {
    if (!discord || !discord->IsReady()) return;

    std::string safeCh, safeTok;
    {
        std::lock_guard<std::mutex> lock(ConfigMutex);
        safeCh = this->AuthChannel;
        safeTok = this->CustomBotToken;
    }

    std::thread([this, player, screenshotUrl, safeCh, safeTok]() {
        std::vector<std::pair<std::string, std::string>> fields = {
            {"Player USN", std::to_string(player->Identity.USN)},
            {"Username", player->Identity.UserName},
            {"In Game Name", player->Identity.InGameName},
            {"Computer Name", player->Identity.ComputerUserName},
            {"Domain Name", player->Identity.ComputerDomainName},
            {"Client IP", player->Identity.UserIP},
            {"Discord ID", player->Identity.DiscrodID},
            {"Hardware UUID", player->Identity.HardwareUUID},
            {"Hardware GUID", player->Identity.HardwareGUID}
        };

        std::string payload = discord->BuildEmbed("Player Login Session", DiscordColors::Maroon, fields, screenshotUrl);
        EnqueueLog(safeCh, payload, safeTok);
        }).detach();
}

void She3aSrv::LogBanToDiscord(std::shared_ptr<PlayerInfo> player, const char* reason, const char* errorCode, const std::string& screenshotUrl) {
    if (!discord || !discord->IsReady()) return;

    std::string safeCh, safeTok;
    {
        std::lock_guard<std::mutex> lock(ConfigMutex);
        safeCh = this->BanChannel;
        safeTok = this->CustomBotToken;
    }

    std::thread([this, player, reason, errorCode, screenshotUrl, safeCh, safeTok]() {
        std::vector<std::pair<std::string, std::string>> fields = {
            {"Target", player->Identity.UserName},
            {"In Game Name", player->Identity.InGameName},
            {"Detection", reason},
            {"Error Code", errorCode},
            {"Client IP", player->Identity.UserIP},
            {"Hardware UUID", player->Identity.HardwareUUID}
        };

        std::string payload = discord->BuildEmbed("Anti-Cheat Punishment Log", DiscordColors::Red, fields, screenshotUrl);
        EnqueueLog(safeCh, payload, safeTok);
        }).detach();
}

void She3aSrv::LogHeartbeatSCToDiscord(std::shared_ptr<PlayerInfo> player, const std::string& screenshotUrl) {
    if (!discord || !discord->IsReady()) return;

    std::string safeCh, safeTok;
    {
        std::lock_guard<std::mutex> lock(ConfigMutex);
        safeCh = this->HeartbeatChannel;
        safeTok = this->CustomBotToken;
    }

    std::thread([this, player, screenshotUrl, safeCh, safeTok]() {
        std::vector<std::pair<std::string, std::string>> fields = {
            {"Player USN", std::to_string(player->Identity.USN)},
            {"Username", player->Identity.UserName},
            {"In Game Name", player->Identity.InGameName},
            {"Client IP", player->Identity.UserIP}
        };

        std::string payload = discord->BuildEmbed("Heartbeat Screenshot Log", DiscordColors::Blue, fields, screenshotUrl);
        EnqueueLog(safeCh, payload, safeTok);
        }).detach();
}

void She3aSrv::UpdateNextScreenshotTime(std::shared_ptr<PlayerInfo> player) {
    if (!player) return;

    int score = player->Security.SuspicionScore.load();
    auto now = std::chrono::steady_clock::now();
    int randomMinutes = 0;

    if (score < 20) {
        randomMinutes = 10 + (rand() % 6);
        player->State.NextRandomScreenshotTime = now + std::chrono::minutes(randomMinutes);
    }
    else if (score <= 50) {
        randomMinutes = 4 + (rand() % 3);
        player->State.NextRandomScreenshotTime = now + std::chrono::minutes(randomMinutes);
    }
    else if (score <= 80) {
        randomMinutes = 1 + (rand() % 3);
        player->State.NextRandomScreenshotTime = now + std::chrono::minutes(randomMinutes);
    }
    else {
        player->State.NextRandomScreenshotTime = now + std::chrono::seconds(30);
    }
}

void She3aSrv::UdpListenLoop() {
    char buffer[65000];
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    std::unordered_map<unsigned long, DWORD> lastLogTimes;

    while (true) {
        int bytesRead = recvfrom(UdpListenSocket, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrSize);
        if (bytesRead <= sizeof(UDP_FRAME_HEADER)) continue;

        UDP_FRAME_HEADER* header = (UDP_FRAME_HEADER*)buffer;

        if (header->PayloadLen < 0 || header->PayloadLen > 60000) continue;

        std::shared_ptr<PlayerInfo> targetPlayer = nullptr;

        {
            std::lock_guard<std::mutex> lock(PlayersMutex);
            for (auto& pair : OnlinePlayers) {
                if (pair.second->Identity.She3aUSN == header->USN) {
                    targetPlayer = pair.second;
                    targetPlayer->UdpEndpoint = senderAddr;
                    targetPlayer->State.CurrentRoomID = header->RoomID;

                    if (header->Type == UDP_TYPE_VOICE) {
                        targetPlayer->State.CurrentTeamID = header->TeamID;
                        targetPlayer->State.CurrentVoiceChannel = header->Channel;
                    }
                    break;
                }
            }
        } 

        if (!targetPlayer) continue;

        DWORD currentTime = GetTickCount();
        bool shouldLog = false;
        if (currentTime - lastLogTimes[header->USN] >= 5000) {
            shouldLog = true;
            lastLogTimes[header->USN] = currentTime;
        }

        if (header->Type == UDP_TYPE_STREAM && targetPlayer->State.LiveStream.isActive) {

            if (shouldLog) {
                this->Log(L_DEBUG, "UDP_Stream", "Player [" + targetPlayer->Identity.InGameName + "] is connected to Live Stream.");
            }

            std::lock_guard<std::mutex> frameLock(targetPlayer->State.LiveStream.frameMutex);

            if (header->FrameID != targetPlayer->State.LiveStream.currentFrameID) {
                targetPlayer->State.LiveStream.currentChunks.clear();
                targetPlayer->State.LiveStream.currentFrameID = header->FrameID;
            }

            std::vector<unsigned char> payloadData(buffer + sizeof(UDP_FRAME_HEADER), buffer + sizeof(UDP_FRAME_HEADER) + header->PayloadLen);
            targetPlayer->State.LiveStream.currentChunks[header->ChunkIdx] = payloadData;

            if (targetPlayer->State.LiveStream.currentChunks.size() == header->MaxChunks) {
                bool isComplete = true;
                std::vector<unsigned char> fullJPEG;
                for (int i = 0; i < header->MaxChunks; i++) {
                    if (targetPlayer->State.LiveStream.currentChunks.find(i) == targetPlayer->State.LiveStream.currentChunks.end()) {
                        isComplete = false; break;
                    }
                    fullJPEG.insert(fullJPEG.end(),
                        targetPlayer->State.LiveStream.currentChunks[i].begin(),
                        targetPlayer->State.LiveStream.currentChunks[i].end());
                }
                if (isComplete) targetPlayer->State.LiveStream.latestJPEG = fullJPEG;
                targetPlayer->State.LiveStream.currentChunks.clear();
            }
        }
        else if (header->Type == UDP_TYPE_VOICE) {

            if (shouldLog) {
                std::string chName = (header->Channel == 0) ? "GLOBAL" : "TEAM";
                std::string talkingState = header->IsTalking ? "TALKING" : "SILENT";
                std::string msg = "Player [" + std::string(header->IGN) + "] on VOIP | Mode: " + chName +
                    " | Team: " + std::to_string(header->TeamID) + " | State: " + talkingState;
                this->Log(L_DEBUG, "UDP_VOIP", msg);
            }

            std::vector<sockaddr_in> targetEndpoints;

            {
                std::lock_guard<std::mutex> lock(PlayersMutex);
                for (auto& pair : OnlinePlayers) {
                    auto otherPlayer = pair.second;

                    if (otherPlayer->Identity.She3aUSN != header->USN &&
                        otherPlayer->State.CurrentRoomID == header->RoomID &&
                        otherPlayer->UdpEndpoint.sin_port != 0)
                    {
                        bool shouldSend = false;

                        if (header->Channel == CH_GLOBAL) {
                            if (otherPlayer->State.CurrentVoiceChannel == CH_GLOBAL)
                                shouldSend = true;
                        }
                        else if (header->Channel == CH_TEAM) {
                            if ((otherPlayer->State.CurrentTeamID == targetPlayer->State.CurrentTeamID) &&
                                (otherPlayer->State.CurrentVoiceChannel == CH_TEAM))
                                shouldSend = true;
                        }

                        if (shouldSend) {
                            targetEndpoints.push_back(otherPlayer->UdpEndpoint);
                        }
                    }
                }
            }

            for (const auto& target : targetEndpoints) {
                sendto(UdpListenSocket, buffer, bytesRead, 0, (sockaddr*)&target, sizeof(target));
            }
        }
    }
}
void She3aSrv::MonitorWorker() {
    She3aSrv* srv = She3aSrv::Instance;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::vector<std::shared_ptr<PlayerInfo>> playersToKick;
        {
            std::lock_guard<std::mutex> lock(srv->PlayersMutex);
            auto it = srv->OnlinePlayers.begin();
            auto now = std::chrono::steady_clock::now();

            while (it != srv->OnlinePlayers.end()) {
                auto player = it->second;

                bool isClearToErase = !player->State.isAwaitingEvidence &&
                    !player->State.isAwaitingAuthEvidence &&
                    !player->State.isAwaitingHeartbeatEvidence;

                if (player->State.IsDisconnected) {
                    if (isClearToErase) {
                        it = srv->OnlinePlayers.erase(it);
                    }
                    else {
                        it++;
                    }
                    continue;
                }

                if (player->Security.IsDetected && !player->State.isAwaitingEvidence) {
                    srv->Log(L_SUCCESS, "Security", "Finalizing punishment for " + player->Identity.UserName + ". Kicking...");
                    playersToKick.push_back(player);
                    it++;
                    continue;
                }

                if (player->State.isAwaitingEvidence) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player->Security.evidenceStartTime).count();
                    if (elapsed >= 7) {
                        srv->Log(L_WARN, "Security", "Evidence Timeout for " + player->Identity.UserName + ". Sending log without image.");
                        srv->LogBanToDiscord(player, player->Security.ErrorMsg.c_str(), player->Security.ErrorCode.c_str(), "Failed to capture image (Timeout)");

                        AC_MSG_PACKET finalAck = { 0 };
                        finalAck.PacketID = SC_RESPONSE_ERROR_REPORT;
                        finalAck.USN = player->Identity.USN;
                        srv->SendPacket(player, (BYTE*)&finalAck, sizeof(AC_MSG_PACKET));

                        player->State.isAwaitingEvidence = false;
                    }
                    it++;
                    continue;
                }

                if (player->State.isAwaitingAuthEvidence) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player->Security.AuthEvidenceStartTime).count();
                    if (elapsed >= 10) {
                        srv->Log(L_WARN, "Security", "Auth Evidence Timeout for " + player->Identity.UserName);
                        srv->LogAuthToDiscord(player, "Failed to capture image (Timeout)");
                        player->State.isAwaitingAuthEvidence = false;
                    }
                }

                bool isBusy = player->State.isExpectingAuth || player->State.isExpectingError ||
                    player->State.isExpectingRoomInfo || player->State.isExpectingHeartbeat ||
                    player->State.isExpectingScreenshot || player->State.isExpectingDiscord;

                if (isBusy) {
                    auto waitSecs = std::chrono::duration_cast<std::chrono::seconds>(now - player->State.LastPulseTime).count();
                    auto lifetime = std::chrono::duration_cast<std::chrono::seconds>(now - player->ConnectionTime).count();

                    if (waitSecs > 60 && lifetime > 15) {
                        srv->Log(L_WARN, "Network", "Timeout: Kicking " + player->Identity.UserIP + " (Non-responsive)");
                        playersToKick.push_back(player);
                        it++;
                        continue;
                    }
                }

                if (player->State.IsAuthorized && now >= player->State.NextRandomScreenshotTime) {
                    srv->UpdateNextScreenshotTime(player);
                    SC_SCREENSHOT_REQUEST scmsg = { 0 };
                    scmsg.PacketID = SC_SCREENSHOT_REQ;
                    scmsg.Operation = HEARTBEAT_REQ;
                    if (srv->SendPacket(player, (BYTE*)&scmsg, sizeof(SC_SCREENSHOT_REQUEST))) {
                        player->State.isExpectingScreenshot = true;
                        player->State.isAwaitingHeartbeatEvidence = true;
                        player->Security.HeartbeatEvidenceStartTime = now;
                    }
                }

                auto idleSecs = std::chrono::duration_cast<std::chrono::seconds>(now - player->State.LastPulseTime).count();
                if (idleSecs >= 30 && !player->State.isExpectingHeartbeat) {
                    player->State.isExpectingHeartbeat = true;
                    AC_MSG_PACKET reqh = { 0 };
                    reqh.PacketID = SC_HEARTBEAT_REQ;
                    reqh.USN = player->Identity.USN;
                    srv->SendPacket(player, (BYTE*)&reqh, sizeof(AC_MSG_PACKET));
                }

                it++;
            }
        }

        for (auto& pToKick : playersToKick) {
            srv->CleanupPlayer(pToKick);
        }
    }
}

void She3aSrv::Run() {
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(ListenSocket, (sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSocket != INVALID_SOCKET) {
            if (ConnectedCnt >= MAX_SERVER_CONNECTIONS) {
                this->Log(L_WARN, "Security", "Server full! Rejecting connection from " + std::string(inet_ntoa(clientAddr.sin_addr)));
                closesocket(clientSocket);
                continue;
            }

            DWORD timeout = 90000;
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
            setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

            this->Log(L_INFO, "Network", "New Connection from: " + std::string(inet_ntoa(clientAddr.sin_addr)));

            std::shared_ptr<PlayerInfo> newPlayer;
            {
                std::lock_guard<std::mutex> lock(PlayersMutex);
                newPlayer = std::make_shared<PlayerInfo>(clientSocket);
                newPlayer->hSocket = clientSocket;
                newPlayer->Identity.UserIP = inet_ntoa(clientAddr.sin_addr);

                OnlinePlayers[clientSocket] = newPlayer;
                ConnectedCnt++;
            }

            AC_MSG_PACKET authReq;
            memset((BYTE*)&authReq, 0, sizeof(AC_MSG_PACKET));
            authReq.PacketID = SC_AUTHDATA_REQ;
            authReq.USN = 0;
            newPlayer->State.isExpectingAuth = true;

            SendPacket(newPlayer, (BYTE*)&authReq, sizeof(AC_MSG_PACKET));

            std::thread t(ClientThread, newPlayer);
            t.detach();
        }
    }
}

void She3aSrv::ClientThread(std::shared_ptr<PlayerInfo> player) {
    BYTE recvBuf[17408];
    She3aSrv* srv = She3aSrv::Instance;
    SOCKET clientSocket = player->hSocket;

    while (true) {
        int bytesRecv = recv(clientSocket, (char*)recvBuf, sizeof(recvBuf), 0);

        if (bytesRecv <= 0) {
            srv->CleanupPlayer(player);
            break;
        }

        auto now_steady = std::chrono::steady_clock::now();
        player->State.LastPulseTime = now_steady;

        if (player->Security.StreamBuffer.size() + bytesRecv > MAX_STREAM_BUFFER) {
            srv->Log(L_WARN, "Security", "StreamBuffer Overflow from " + player->Identity.UserIP + ". Kicking...");
            srv->CleanupPlayer(player);
            break;
        }

        player->Security.StreamBuffer.insert(player->Security.StreamBuffer.end(), recvBuf, recvBuf + bytesRecv);

        while (player->Security.StreamBuffer.size() >= sizeof(PACKET_HEADER)) {

            BYTE tempHeaderBuf[sizeof(PACKET_HEADER)];
            memcpy(tempHeaderBuf, player->Security.StreamBuffer.data(), sizeof(PACKET_HEADER));
            srv->DecryptDatas(tempHeaderBuf, sizeof(PACKET_HEADER));

            PACKET_HEADER* header = (PACKET_HEADER*)tempHeaderBuf;

            auto now_sys = std::chrono::system_clock::now();
            unsigned long long currentSrvTime = std::chrono::duration_cast<std::chrono::seconds>(now_sys.time_since_epoch()).count();

            unsigned long long diff = (currentSrvTime > header->Timestamp) ?
                (currentSrvTime - header->Timestamp) : (header->Timestamp - currentSrvTime);

            if (diff > 120) {
                srv->Log(L_WARN, "Security", "Expired Timestamp Detected! Diff: " + std::to_string(diff) + "s from " + player->Identity.UserIP + ". Kicking...");
                srv->CleanupPlayer(player);
                return;
            }

            if (header->Timestamp < player->Security.lastReceivedTimestamp && player->Security.lastReceivedTimestamp != 0) {
                srv->Log(L_WARN, "Security", "Packet Replay Detected (Backwards Time) from " + player->Identity.UserIP + ". Kicking...");
                srv->CleanupPlayer(player);
                return;
            }
            player->Security.lastReceivedTimestamp = header->Timestamp;

            if (header->Len > MAX_ALLOWED_CHUNK) {
                srv->Log(L_WARN, "Security", "Illegal Chunk Length (" + std::to_string(header->Len) + ") from " + player->Identity.UserIP);
                srv->CleanupPlayer(player);
                return;
            }

            size_t fullPacketSize = sizeof(PACKET_HEADER) + header->Len;

            if (player->Security.StreamBuffer.size() >= fullPacketSize) {
                std::vector<BYTE> fullPacket(fullPacketSize);
                memcpy(fullPacket.data(), player->Security.StreamBuffer.data(), fullPacketSize);
                player->Security.StreamBuffer.erase(player->Security.StreamBuffer.begin(), player->Security.StreamBuffer.begin() + fullPacketSize);
                srv->DecryptDatas(fullPacket.data(), fullPacketSize);
                srv->HandlePacket(player, fullPacket.data(), (int)fullPacketSize);
            }
            else {
                break;
            }
        }
    }
}

void She3aSrv::HandlePacket(std::shared_ptr<PlayerInfo> player, BYTE* rData, int prelen) {
    PACKET_HEADER* workheader = (PACKET_HEADER*)rData;
    auto now = std::chrono::steady_clock::now();

    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - player->Security.lastRateReset).count();
    if (elapsedMs >= 1000) {
        player->Security.packetRateCount = 0;
        player->Security.lastRateReset = now;
    }

    player->Security.packetRateCount++;
    if (player->Security.packetRateCount > MAX_PACKETS_PER_SECOND) {
        this->Log(L_WARN, "Security", "Rate Limit Exceeded (PPS) from " + player->Identity.UserIP + ". Kicking...");
        this->CleanupPlayer(player);
        return;
    }

    unsigned long computedCRC = CRC32::Compute(rData + sizeof(PACKET_HEADER), workheader->Len);
    if (workheader->CRC != computedCRC) {
        this->Log(L_WARN, "Security", "CRC Mismatch! PacketID: " + std::to_string((int)workheader->PacketID) + " IP: " + player->Identity.UserIP);
        return;
    }

    if (workheader->ExtCount == 0) {
        this->ProcessFinalPacket(player, rData, prelen);
    }
    else {
        if (player->Security.reassemblyMap.size() > 5) {
            this->Log(L_WARN, "Security", "Too many concurrent reassemblies from " + player->Identity.UserIP);
            this->CleanupPlayer(player);
            return;
        }

        size_t maxReassemblySize = 17 * 1024;
        switch (workheader->PacketID) {
        case CS_SCREENSHOT_DATA:   maxReassemblySize = 5 * 1024 * 1024; break;
        case CS_AUTH_REQ:          maxReassemblySize = 17 * 1024; break;
        }

        auto& storage = player->Security.reassemblyMap[workheader->PacketID];

        if (workheader->CurCount == 1) {
            storage.clear();
            player->Security.assemblyStartTime = now;
        }

        unsigned short expectedChunkNumber = (unsigned short)((storage.size() / MAX_SRV_CHUNK_SIZE) + 1);
        if (workheader->CurCount != expectedChunkNumber) {
            this->Log(L_WARN, "Security", "Out-of-order Chunk Detected! Expected: " + std::to_string(expectedChunkNumber) + " Got: " + std::to_string(workheader->CurCount) + " from " + player->Identity.UserIP);
            storage.clear();
            this->CleanupPlayer(player);
            return;
        }

        auto totalElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player->Security.assemblyStartTime).count();
        auto interElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player->Security.lastChunkTime).count();

        if (totalElapsed > 20 || (workheader->CurCount > 1 && interElapsed > 5)) {
            this->Log(L_WARN, "Security", "Reassembly Timeout for " + player->Identity.UserIP);
            storage.clear();
            return;
        }

        if (storage.size() + workheader->Len > maxReassemblySize) {
            this->Log(L_WARN, "Security", "Reassembly Cap Exceeded for PacketID: " + std::to_string((int)workheader->PacketID) + " IP: " + player->Identity.UserIP);
            storage.clear();
            this->CleanupPlayer(player);
            return;
        }

        storage.insert(storage.end(), rData + sizeof(PACKET_HEADER), rData + sizeof(PACKET_HEADER) + workheader->Len);
        player->Security.lastChunkTime = now;

        if (workheader->CurCount == workheader->ExtCount) {
            std::vector<BYTE> finalBuf(sizeof(PACKET_HEADER) + storage.size());
            memcpy(finalBuf.data(), rData, sizeof(PACKET_HEADER));

            PACKET_HEADER* fh = (PACKET_HEADER*)finalBuf.data();
            fh->Len = (unsigned int)storage.size();
            fh->CurCount = 0; fh->ExtCount = 0;

            memcpy(finalBuf.data() + sizeof(PACKET_HEADER), storage.data(), storage.size());
            storage.clear();

            if (!this->ProcessFinalPacket(player, finalBuf.data(), (int)finalBuf.size()))
                this->CleanupPlayer(player);
        }
    }
}

bool She3aSrv::ProcessFinalPacket(std::shared_ptr<PlayerInfo> player, BYTE* rawData, int len) {
    PACKET_HEADER* header = (PACKET_HEADER*)rawData;

    switch (header->PacketID) {
    case CS_AUTH_REQ:
    {
        if (!player->State.isExpectingAuth) return false;
        if (len != sizeof(SEND_AUTH_REQUEST)) return false;

        Instance->Log(L_DEBUG, "Network", "Received Auth Request from " + player->Identity.UserIP);

        SEND_AUTH_REQUEST* authRequest = (SEND_AUTH_REQUEST*)rawData;
        player->Identity.USN = authRequest->USN - 21;
        player->Identity.She3aUSN = authRequest->USN;
        player->Identity.UserName = authRequest->LoginID;
        player->Identity.InGameName = authRequest->UserIGN;
        player->Identity.Password = authRequest->Password;
        player->Identity.EncryptedPassword = Instance->ComputeMD5Hash(player->Identity.Password + SystemSalt);
        player->Identity.ComputerUserName = authRequest->UserName;
        player->Identity.ComputerDomainName = authRequest->ComputerName;
        player->Identity.DiscrodID = authRequest->DiscordID;
        player->Identity.HardwareUUID = authRequest->HardwareUUID;
        player->Identity.HardwareGUID = authRequest->HardwareGUID;
        player->Identity.Authority = Instance->DB_GetAuthority(player->Identity.USN);
        player->State.IsAuthorized = Instance->DB_ExecuteAuth(player);
        player->Security.SuspicionScore = Instance->db->GetSavedScore(player->Identity.HardwareGUID.c_str());

        if (player->State.IsAuthorized)
        {
            AUTH_REQUEST_RESPONSE response;
            memset((BYTE*)&response, 0, sizeof(AUTH_REQUEST_RESPONSE));
            response.PacketID = SC_RESPONSE_AUTHDATA;
            response.USN = player->Identity.USN;
            response.PlayerType = player->Identity.Authority;

            if (authRequest->CurVersion != acVersion) {
                response.Version = 0xDEFEC8ED;
            }
            else
            {
                response.Version = acVersion;
            }

            SendPacket(player, (BYTE*)&response, sizeof(AUTH_REQUEST_RESPONSE));
            Sleep(5);
            SC_SCREENSHOT_REQUEST scmsg;
            memset((BYTE*)&scmsg, 0, sizeof(SC_SCREENSHOT_REQUEST));
            scmsg.PacketID = SC_SCREENSHOT_REQ;
            scmsg.Operation = NORMAL_REQ;
            if (SendPacket(player, (BYTE*)&scmsg, sizeof(SC_SCREENSHOT_REQUEST)))
                player->State.isExpectingScreenshot = true;
            player->State.isAwaitingAuthEvidence = true;
            player->Security.AuthEvidenceStartTime = std::chrono::steady_clock::now();
        }
        player->State.isExpectingAuth = false;
        break;
    }
    case CS_ERROR_REPORT_REQ:
    {
        Instance->Log(L_DEBUG, "Network", "Received Error Report Request from " + player->Identity.UserIP);
        AC_MSG_PACKET approve;
        memset((BYTE*)&approve, 0, sizeof(AC_MSG_PACKET));
        approve.PacketID = SC_APPROVE_ERROR;
        approve.USN = player->Identity.USN;
        if (SendPacket(player, (BYTE*)&approve, sizeof(AC_MSG_PACKET)))
            player->State.isExpectingError = true;
        break;
    }
    case CS_ROOMINFO_REPORT_REQ:
    {
        Instance->Log(L_DEBUG, "Network", "Received Room Info Request from " + player->Identity.UserIP);
        AC_MSG_PACKET approve;
        memset((BYTE*)&approve, 0, sizeof(AC_MSG_PACKET));
        approve.PacketID = SC_APPROVE_ROOMINFO;
        approve.USN = player->Identity.USN;
        if (SendPacket(player, (BYTE*)&approve, sizeof(AC_MSG_PACKET)))
            player->State.isExpectingRoomInfo = true;
        break;
    }
    case CS_ERROR_REPORT:
    {
        if (!player->State.isExpectingError) {
            Instance->Log(L_WARN, "Network", "Unexpected Error Report from " + player->Identity.UserIP);
            return false;
        }

        if (len != sizeof(SEND_ERROR_REPORT)) return false;

        SEND_ERROR_REPORT* errorRpt = (SEND_ERROR_REPORT*)rawData;
        Instance->Log(L_WARN, "Detection", "Received Error Report from " + player->Identity.UserIP + " | Code: " + errorRpt->ErrorCode);

        if (errorRpt->USN == player->Identity.She3aUSN) {
            player->Security.IsDetected = true;
            player->Security.ErrorCode = errorRpt->ErrorCode;
            player->Security.ErrorMsg = errorRpt->ErrorMsg;
            DetectedCnt++;

            Instance->DB_ExecuteBan(player, player->Security.ErrorCode.c_str(), player->Security.ErrorMsg.c_str(), "Awaiting Evidence...");

            SC_SCREENSHOT_REQUEST scmsg;
            memset((BYTE*)&scmsg, 0, sizeof(SC_SCREENSHOT_REQUEST));
            scmsg.PacketID = SC_SCREENSHOT_REQ;
            scmsg.Operation = REPORT_REQ;
            if (SendPacket(player, (BYTE*)&scmsg, sizeof(SC_SCREENSHOT_REQUEST))) {
                player->State.isExpectingScreenshot = true;
                player->State.isAwaitingEvidence = true;
                player->Security.evidenceStartTime = std::chrono::steady_clock::now();
            }
        }
        player->State.isExpectingError = false;
        break;
    }
    case CS_ROOMINFO_REPORT:
    {


        //Disabled and not impelmented for now
        break;
    }
    case CS_HEARTBEAT_MSG:
    {
        if (!player->State.isExpectingHeartbeat) return true;
        Instance->Log(L_DEBUG, "Network", "Received Heartbeat Message from " + player->Identity.UserIP);
        player->State.isExpectingHeartbeat = false;
        break;
    }
    case CS_SCREENSHOT_DATA: {
        if (!player->State.isExpectingScreenshot) {
            Instance->Log(L_WARN, "Security", "Received unexpected or duplicate screenshot from " + player->Identity.UserIP + ". Ignoring.");
            return true;
        }

        Instance->Log(L_DEBUG, "Network", "Received Screenshot Data from " + player->Identity.UserIP);

        SEND_SCREENSHOT* ssHeader = (SEND_SCREENSHOT*)rawData;
        unsigned int imageSize = ssHeader->ScreenshotSize;
        BYTE* imageData = rawData + sizeof(SEND_SCREENSHOT);

        if (len < sizeof(SEND_SCREENSHOT) + imageSize) return false;

        std::string scLink = Instance->SaveScreenshot(player, imageData, imageSize, ssHeader->Operation);

        switch (ssHeader->Operation) {
        case NORMAL_REQ:
            player->State.NormalScreenshotTime = std::chrono::steady_clock::now();
            player->State.RequestDiscordDataTrigger = true;
            Instance->LogAuthToDiscord(player, scLink);
            player->State.isAwaitingAuthEvidence = false;
            break;

        case REPORT_REQ: {
            player->State.BanScreenshotTime = std::chrono::steady_clock::now();
            Instance->DB_UpdateProofUrl(player, scLink.c_str());
            Instance->LogBanToDiscord(player, player->Security.ErrorMsg.c_str(), player->Security.ErrorCode.c_str(), scLink);
            AC_MSG_PACKET finalAck;
            memset((BYTE*)&finalAck, 0, sizeof(AC_MSG_PACKET));
            finalAck.PacketID = SC_RESPONSE_ERROR_REPORT;
            finalAck.USN = player->Identity.USN;
            SendPacket(player, (BYTE*)&finalAck, sizeof(AC_MSG_PACKET));
            player->State.isAwaitingEvidence = false;
            break;
        }

        case HEARTBEAT_REQ:
            player->State.HeartBeatScreenshotTime = std::chrono::steady_clock::now();
            Instance->LogHeartbeatSCToDiscord(player, scLink);
            player->State.isAwaitingHeartbeatEvidence = false;
            break;
        }

        UpdateNextScreenshotTime(player);
        player->State.isExpectingScreenshot = false;
        break;
    }
    default:
        Instance->Log(L_WARN, "Network", "Unknown Packet ID: " + std::to_string((int)header->PacketID) + " from " + player->Identity.UserIP);
        break;
    }
    return true;
}

bool She3aSrv::SendPacket(std::shared_ptr<PlayerInfo> player, BYTE* data, size_t len) {
    if (!player || player->hSocket == INVALID_SOCKET) return false;

    DWORD waitStart = GetTickCount();
    const DWORD MAX_WAIT_MS = 15000;

    while (player->State.IsSendingData) {
        if (GetTickCount() - waitStart > MAX_WAIT_MS) {
            player->State.IsSendingData = false;
            break;
        }
        Sleep(2);
    }

    player->State.IsSendingData = true;

    auto now = std::chrono::system_clock::now();
    unsigned long long currentTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    PACKET_HEADER* originalHeader = (PACKET_HEADER*)data;
    size_t headerSize = sizeof(PACKET_HEADER);
    size_t payloadTotalSize = len - headerSize;

    if (len <= (MAX_SRV_CHUNK_SIZE + headerSize)) {
        originalHeader->CurCount = 0;
        originalHeader->ExtCount = 0;
        originalHeader->Timestamp = currentTimestamp;
        originalHeader->Len = (unsigned int)payloadTotalSize;
        originalHeader->CRC = CRC32::Compute(data + headerSize, (unsigned int)payloadTotalSize);

        EncryptDatas(data, len);

        int sentBytes = send(player->hSocket, (const char*)data, (int)len, 0);
        if (sentBytes == SOCKET_ERROR) {
            player->State.IsSendingData = false;
            CleanupPlayer(player);
            return false;
        }
    }
    else {
        unsigned short totalChunks = (unsigned short)((payloadTotalSize + MAX_SRV_CHUNK_SIZE - 1) / MAX_SRV_CHUNK_SIZE);
        BYTE* payloadPtr = data + headerSize;
        size_t remainingSize = payloadTotalSize;

        for (unsigned short i = 1; i <= totalChunks; i++) {
            size_t currentChunkSize = (remainingSize > MAX_SRV_CHUNK_SIZE) ? MAX_SRV_CHUNK_SIZE : remainingSize;
            size_t totalPacketSize = headerSize + currentChunkSize;

            std::vector<BYTE> chunk(totalPacketSize);
            PACKET_HEADER* ch = (PACKET_HEADER*)chunk.data();

            ch->PacketID = originalHeader->PacketID;
            ch->CurCount = i;
            ch->ExtCount = totalChunks;
            originalHeader->Timestamp = currentTimestamp;
            ch->Len = (unsigned int)currentChunkSize;

            memcpy(chunk.data() + headerSize, payloadPtr, currentChunkSize);
            ch->CRC = CRC32::Compute(chunk.data() + headerSize, (unsigned int)currentChunkSize);

            EncryptDatas(chunk.data(), totalPacketSize);

            int sent = send(player->hSocket, (const char*)chunk.data(), (int)totalPacketSize, 0);
            if (sent == SOCKET_ERROR) {
                player->State.IsSendingData = false;
                CleanupPlayer(player);
                return false;
            }

            payloadPtr += currentChunkSize;
            remainingSize -= currentChunkSize;
        }
    }

    player->State.IsSendingData = false;
    return true;
}

void She3aSrv::CleanupPlayer(std::shared_ptr<PlayerInfo> targetPlayer) {
    std::lock_guard<std::mutex> lock(PlayersMutex);

    for (auto it = OnlinePlayers.begin(); it != OnlinePlayers.end(); ++it) {
        if (it->second == targetPlayer) {
            auto player = it->second;

            if (player->State.IsDisconnected) {
                return;
            }

            bool hasPendingEvidence = player->State.isAwaitingEvidence ||
                player->State.isAwaitingAuthEvidence ||
                player->State.isAwaitingHeartbeatEvidence;

            if (hasPendingEvidence) {
                player->State.IsDisconnected = true;
                if (player->hSocket != INVALID_SOCKET) closesocket(player->hSocket);
                player->hSocket = INVALID_SOCKET;
                ConnectedCnt--;
            }
            else {
                if (player->hSocket != INVALID_SOCKET) closesocket(player->hSocket);
                ConnectedCnt--;
                OnlinePlayers.erase(it);
            }
            return;
        }
    }
}