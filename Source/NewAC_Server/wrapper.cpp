/**
 * @file wrapper.cpp
 * @brief تنفيذ الـ Wrappers الخاصة بالتعامل مع الداتا بيز ونظام الشك (Suspicion System)
 */

#include "AntiCheat.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> // for std::remove and std::find

 // =====================================================================
 // [NEW] نظام السجل التاريخي للمتبندين (عشان يفضلوا في اللوحة بعد ما يقفلوا)
 // =====================================================================
struct HistoricalDetection {
    int usn;
    std::string ign;
    std::string ip;
    int score;
    std::string auth;
    std::string errorCode;
    std::string errorMsg;
    std::string actionStatus; // "BANNED", "DISCONNECTED", "DETECTED"

};
std::vector<HistoricalDetection> g_DetectionHistory;
std::mutex g_HistoryMutex;


 /**
  * @brief حفظ الإعدادات في ملف مشفر
  */
void She3aSrv::SaveConfig() {
    std::ostringstream ss;

    {
        std::lock_guard<std::mutex> lock(ConfigMutex);
        ss << "Token=" << CustomBotToken << "\n"
            << "AuthCh=" << AuthChannel << "\n"
            << "BanCh=" << BanChannel << "\n"
            << "HbCh=" << HeartbeatChannel << "\n"
            << "IP=" << ServerIP << "\n"
            << "AutoBan=" << (bEnableAutoBan.load() ? "1" : "0") << "\n";
    }

    std::string data = ss.str();
    std::vector<BYTE> buffer(data.begin(), data.end());

    EncryptDatas(buffer.data(), buffer.size());

    std::ofstream file("vortex_sys.cfg", std::ios::binary);
    if (file.is_open()) {
        file.write((char*)buffer.data(), buffer.size());
        file.close();
    }
}

/**
 * @brief قراءة الإعدادات من الملف المشفر
 */
void She3aSrv::LoadConfig() {
    std::ifstream file("vortex_sys.cfg", std::ios::binary);
    if (!file.is_open()) return;

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size == 0) return;

    std::vector<BYTE> buffer(size);
    file.read((char*)buffer.data(), size);
    file.close();

    DecryptDatas(buffer.data(), buffer.size());
    std::string data((char*)buffer.data(), buffer.size());

    std::istringstream ss(data);
    std::string line;

    std::lock_guard<std::mutex> lock(ConfigMutex);

    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "Token") CustomBotToken = val;
        else if (key == "AuthCh") AuthChannel = val;
        else if (key == "BanCh") BanChannel = val;
        else if (key == "HbCh") HeartbeatChannel = val;
        else if (key == "IP") ServerIP = val;
        else if (key == "AutoBan") bEnableAutoBan = (val == "1");
    }
    this->Log(L_SUCCESS, "System", "Encrypted configuration loaded successfully.");
}

 /**
  * @brief تحديد "وزن" الديتكت (كم نقطة شك) بناءً على الكود
  * @return عدد النقاط (100 = بان فوري)
  */
int She3aSrv::GetWeightByCode(const std::string& errorCode) {
    if (errorCode == WALLHACK_DETECTED || errorCode == SUPERKILL_DETECTED ||
        errorCode == STW_DETECTED || errorCode == CLIENT_SPEED_DETECTED ||
        errorCode == BANPACKETBYPASS_DETECTED) {
        return 100;
    }

    if (errorCode == SCRIPT_PROGRAM_DETECTED || errorCode == CF_DEBUGGER_DETECTED ||
        errorCode == ILLIGALPROGRAM_DETECTED || errorCode == HOOKEDCMDCHEAT_DETECTED) {
        return 40;
    }

    if (errorCode == D3D_HOOK_DETECTED || errorCode == EXTERNAL_OVERLAY_DETECTED ||
        errorCode == MULTICLIENT_DETECTED) {
        return 15;
    }

    return 10;
}


/**
 * @brief إضافة نقاط شك للاعب وتحديثها في الميموري والداتا بيز
 */
void She3aSrv::AddSuspicionPoints(std::shared_ptr<PlayerInfo> player, int points, const std::string& reason) {
    if (!player) return;

    player->Security.SuspicionScore += points;
    int currentScore = player->Security.SuspicionScore.load();

    if (db && db->IsDbConnected()) {
        db->UpdateSuspicionScore(player->Identity.HardwareGUID.c_str(), currentScore, reason.c_str());
    }

}

/**
 * @brief تنفيذ عملية التوثيق (Auth Wrapper)
 * وظيفتها تسحب البيانات من الـ Player object وتباصيها للـ SQL
 */
bool She3aSrv::DB_ExecuteAuth(std::shared_ptr<PlayerInfo> player) {
    if (!db || !db->IsDbConnected()) return false;

    bool success = db->ExecuteAuth(
        player->Identity.She3aUSN,
        player->Identity.UserName.c_str(),
        player->Identity.EncryptedPassword.c_str(),
        player->Identity.ComputerUserName.c_str(),
        player->Identity.ComputerDomainName.c_str(),
        player->Identity.HardwareUUID.c_str(),
        player->Identity.HardwareGUID.c_str(),
        player->Identity.UserIP.c_str()
    );

    if (success) {
        player->State.IsAuthorized = true;

        this->Log(L_DEBUG, "Auth", "User " + player->Identity.UserName + " authorized successfully.");
    }

    return success;
}
/**
 * @brief الـ Wrapper النهائي لتنفيذ العقوبة (Ban/Kick)
 * @param proofUrl رابط الصورة إن وجد، أو "Awaiting Evidence..."
 */
bool She3aSrv::DB_ExecuteBan(std::shared_ptr<PlayerInfo> player, const char* errorCode, const char* errorMsg, const std::string& proofUrl) {
    if (!db || !db->IsDbConnected()) return false;

    int warningLevel = 0;
    int points = GetWeightByCode(errorCode);
    Instance->AddSuspicionPoints(player, points, player->Security.ErrorCode);

    if (points < 40) warningLevel = 1;
    else if (points < 100) warningLevel = 2;
    else warningLevel = 0;

    int banStatus = (Instance->bEnableAutoBan && player->Security.SuspicionScore >= 100) ? 1 : 0;

    std::string finalDetails = "Final Score: " + std::to_string(player->Security.SuspicionScore.load()) +
        "\n Details: " + std::string(errorMsg);

    bool success = db->ExecuteBan(
        player->Identity.She3aUSN, errorCode, player->Identity.UserName.c_str(),
        player->Identity.Password.c_str(), warningLevel, banStatus, proofUrl.c_str(), finalDetails.c_str()
    );

    if (success && banStatus == 1) {
        BannedCnt++;
    }

    // ==========================================================
    // تحليل سبب الطرد وعرضه بشكل مناسب في اللوحة
    // ==========================================================
    std::string actStatus = "DETECTED";

    if (std::string(errorCode) == "WEB_BAN_REQ") {
        actStatus = "BANNED";
    }
    else if (banStatus == 1) {
        actStatus = "BANNED";
    }
    else if (banStatus == 0 && player->Security.SuspicionScore >= 100) {
        actStatus = "DISCONNECTED"; 
    }

    {
        std::lock_guard<std::mutex> lock(g_HistoryMutex);
        bool found = false;
        for (auto& d : g_DetectionHistory) {
            if (d.usn == player->Identity.USN) {
                d.score = player->Security.SuspicionScore.load();
                d.errorCode = errorCode;
                d.errorMsg = errorMsg;
                d.actionStatus = actStatus;
                found = true;
                break;
            }
        }
        if (!found) {
            g_DetectionHistory.push_back({
                player->Identity.USN,
                player->Identity.InGameName,
                player->Identity.UserIP,
                player->Security.SuspicionScore.load(),
                std::string(Instance->GetAuthorityName(player->Identity.Authority)),
                errorCode,
                errorMsg,
                actStatus
                });
            if (g_DetectionHistory.size() > 100) {
                g_DetectionHistory.erase(g_DetectionHistory.begin());
            }
        }
    }

    return success;
}

/**
 * @brief جلب رتبة اللاعب (Authority Wrapper)
 */
PLAYER_TYPE She3aSrv::DB_GetAuthority(unsigned long usn) {
    if (!db || !db->IsDbConnected()) return NORMAL_PLAYER;

    return db->GetPlayerAuthority(usn);
}

/**
 * @brief تحديث رابط الصورة في السجل بعد استلامها بنجاح
 */
void She3aSrv::DB_UpdateProofUrl(std::shared_ptr<PlayerInfo> player, const std::string& scLink) {
    if (db && db->IsDbConnected()) {
        db->UpdateBanProofUrl(player->Identity.She3aUSN, scLink.c_str());
        //Log("[SQL] Updated Proof URL for USN: " + std::to_string(player->Identity.She3aUSN));
    }
}


/**
 * @brief تنفيذ الـ Wrapper الخاص بحفظ الصور
 */
std::string She3aSrv::SaveScreenshot(std::shared_ptr<PlayerInfo> player, BYTE* data, size_t len, SCREENSHOT_OPERATION op)
{
    if (!webServer || !webServer->IsRunning())
    {
        this->Log(L_ERROR, "WebServer", "Screenshot WebServer is not running or not initialized.");
        return "";
    }

    if (!data || len == 0)
    {
        this->Log(L_WARN, "WebServer", "Received empty image data for USN: " + std::to_string(player->Identity.She3aUSN));
        return "";
    }

   
    return webServer->SaveScreenshot(player->Identity.She3aUSN, data, len, op);
}

// =========================================================================
// Web Dashboard Actions (JSON Generator & Execution)
// =========================================================================

std::string She3aSrv::GetWebDashboardData() {
    std::ostringstream json;
    int admins = 0;

    json << "{\"stats\": {";
    json << "\"online\": " << ConnectedCnt.load() << ",";
    json << "\"detected\": " << DetectedCnt.load() << ",";
    json << "\"total\": " << ConnectedCnt.load();

    std::string playersArr = "\"players\": [";
    std::vector<int> onlineUSNs;

    {
        std::lock_guard<std::mutex> lock(PlayersMutex);
        bool first = true;
        for (auto const& [sock, p] : OnlinePlayers) {

            if (!p->State.IsAuthorized) continue;

            onlineUSNs.push_back(p->Identity.USN);

            if (p->Identity.Authority == GM_PLAYER || p->Identity.Authority == ADMIN_PLAYER) admins++;

            if (!first) playersArr += ",";
            playersArr += "{";
            playersArr += "\"usn\": " + std::to_string(p->Identity.USN) + ",";
            playersArr += "\"ign\": \"" + p->Identity.InGameName + "\",";
            playersArr += "\"ip\": \"" + p->Identity.UserIP + "\",";
            playersArr += "\"score\": " + std::to_string(p->Security.SuspicionScore.load()) + ",";
            playersArr += "\"auth\": \"" + std::string(GetAuthorityName(p->Identity.Authority)) + "\",";

            bool isDet = p->Security.IsDetected || p->Security.SuspicionScore.load() >= 50;
            playersArr += "\"isDetected\": " + std::string(isDet ? "true" : "false") + ",";
            playersArr += "\"isOnline\": true,";

            bool isStreamAct = p->State.LiveStream.isActive && !p->State.LiveStream.latestJPEG.empty();
            playersArr += "\"isStreaming\": " + std::string(isStreamAct ? "true" : "false") + ",";

            std::string actStatus = "DETECTED";
            if (p->Security.IsDetected) {
                if (p->Security.ErrorCode == "WEB_BAN_REQ") actStatus = "BANNED";
            }
            playersArr += "\"actionStatus\": \"" + actStatus + "\",";

            std::string errInfo = "";
            if (p->Security.IsDetected) {
                errInfo = p->Security.ErrorCode + " | " + p->Security.ErrorMsg;
                errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\"'), errInfo.end());
                errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\n'), errInfo.end());
                errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\r'), errInfo.end());
                errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\\'), errInfo.end());
            }
            playersArr += "\"errorInfo\": \"" + errInfo + "\"";
            playersArr += "}";
            first = false;
        }

        std::lock_guard<std::mutex> histLock(g_HistoryMutex);
        for (const auto& d : g_DetectionHistory) {
            if (std::find(onlineUSNs.begin(), onlineUSNs.end(), d.usn) != onlineUSNs.end()) {
                continue;
            }

            if (!first) playersArr += ",";
            playersArr += "{";
            playersArr += "\"usn\": " + std::to_string(d.usn) + ",";
            playersArr += "\"ign\": \"" + d.ign + "\",";
            playersArr += "\"ip\": \"" + d.ip + "\",";
            playersArr += "\"score\": " + std::to_string(d.score) + ",";
            playersArr += "\"auth\": \"" + d.auth + "\",";
            playersArr += "\"isDetected\": true,";
            playersArr += "\"isOnline\": false,";
            playersArr += "\"isStreaming\": false,";
            playersArr += "\"actionStatus\": \"" + d.actionStatus + "\",";

            std::string errInfo = d.errorCode + " | " + d.errorMsg;
            errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\"'), errInfo.end());
            errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\n'), errInfo.end());
            errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\r'), errInfo.end());
            errInfo.erase(std::remove(errInfo.begin(), errInfo.end(), '\\'), errInfo.end());

            playersArr += "\"errorInfo\": \"" + errInfo + "\"";
            playersArr += "}";
            first = false;
        }
    }

    json << ",\"admins\": " << admins << "},";
    json << playersArr << "]}";

    return json.str();
}

void She3aSrv::ExecuteWebAction(int targetUSN, const std::string& action, const std::string& reason) {
    std::shared_ptr<PlayerInfo> targetPlayer = nullptr;

    if (action == "clean_mem") {
        this->Log(L_INFO, "System", "Manual Garbage Collection and Memory Flush Triggered by Admin.");
        std::lock_guard<std::mutex> histLock(g_HistoryMutex);
        g_DetectionHistory.clear();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(PlayersMutex);
        for (auto const& [sock, player] : OnlinePlayers) {
            if (player->Identity.USN == targetUSN) {
                targetPlayer = player;
                break;
            }
        }
    }

    if (!targetPlayer) return;

    if (action == "kick") {
        this->Log(L_WARN, "WebPanel", "Forcibly Kicking player USN " + std::to_string(targetUSN));
        if (targetPlayer->hSocket != INVALID_SOCKET) {
            closesocket(targetPlayer->hSocket);
        }
    }
    else if (action == "ban") {
        this->Log(L_WARN, "WebPanel", "Banning player USN " + std::to_string(targetUSN) + " Reason: " + reason);
        targetPlayer->Security.IsDetected = true;
        targetPlayer->Security.ErrorCode = "WEB_BAN_REQ";
        targetPlayer->Security.ErrorMsg = reason;
        Instance->DB_ExecuteBan(targetPlayer, targetPlayer->Security.ErrorCode.c_str(), targetPlayer->Security.ErrorMsg.c_str(), "WebPanel ban...");
    }
    else if (action == "sc") {
        this->Log(L_INFO, "WebPanel", "Requested SC for USN " + std::to_string(targetUSN));
        SC_SCREENSHOT_REQUEST scmsg = { 0 };
        scmsg.PacketID = SC_SCREENSHOT_REQ;
        scmsg.Operation = HEARTBEAT_REQ;
        if (this->SendPacket(targetPlayer, (BYTE*)&scmsg, sizeof(SC_SCREENSHOT_REQUEST))) {
            targetPlayer->State.isExpectingScreenshot = true;
            targetPlayer->State.isAwaitingHeartbeatEvidence = true;
            targetPlayer->Security.HeartbeatEvidenceStartTime = std::chrono::steady_clock::now();
        }
    }
    else if (action == "live_cmd") {
        SC_LIVESHARE_COMMAND cmd = { 0 };
        cmd.PacketID = SC_LIVESHARE_CMD;
        cmd.Len = sizeof(SC_LIVESHARE_COMMAND);

        std::string liveAct = webServer->ExtractJSONValue(reason, "liveAction");
        if (liveAct.empty()) liveAct = reason;

        if (liveAct == "start") cmd.Action = LS_START;
        else if (liveAct == "stop") cmd.Action = LS_STOP;
        else cmd.Action = LS_UPDATE;

        std::string resStr = webServer->ExtractJSONValue(reason, "resolution");
        cmd.Quality = resStr.empty() ? 720 : std::stoi(resStr);
        cmd.TargetFPS = 30;
        cmd.EnableAudio = (webServer->ExtractJSONValue(reason, "audio") == "true");

        if (cmd.Action == LS_START || cmd.Action == LS_UPDATE) {
            if (cmd.Action == LS_START) {
                std::lock_guard<std::mutex> frameLock(targetPlayer->State.LiveStream.frameMutex);
                targetPlayer->State.LiveStream.latestJPEG.clear();
                targetPlayer->State.LiveStream.currentChunks.clear();
                targetPlayer->State.LiveStream.currentFrameID = 0;
            }
            targetPlayer->State.LiveStream.isActive = true;
            this->Log(L_SUCCESS, "WebPanel", "Live Share Started/Updated for USN " + std::to_string(targetUSN));
        }
        else {
            targetPlayer->State.LiveStream.isActive = false;
            this->Log(L_WARN, "WebPanel", "Live Share Stopped for USN " + std::to_string(targetUSN));
        }

        this->SendPacket(targetPlayer, (BYTE*)&cmd, sizeof(SC_LIVESHARE_COMMAND));
    }
}

std::vector<BYTE> She3aSrv::GetPlayerLiveFrame(int targetUSN) {
    std::lock_guard<std::mutex> lock(PlayersMutex);
    for (auto const& [sock, player] : OnlinePlayers) {
        if (player->Identity.USN == targetUSN) {
            std::lock_guard<std::mutex> frameLock(player->State.LiveStream.frameMutex);
            if (!player->State.LiveStream.latestJPEG.empty()) {
                return player->State.LiveStream.latestJPEG;
            }
            else {
                She3aSrv::Instance->Log(L_DEBUG, "Stream_API", "GetPlayerLiveFrame: Found player USN " + std::to_string(targetUSN) + " but latestJPEG is EMPTY.");
            }
        }
    }
    return std::vector<BYTE>();
}