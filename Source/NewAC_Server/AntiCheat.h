#pragma once

/**
 * @file She3aSrv.h
 * @brief تعريف كلاس السيرفر الأساسي (Declarations Only)
 */
#include <winsock2.h>
#include "windows.h"
#include <string>
#include <iostream>
#include "PlayerInfo.h"
#include <unordered_map>
#include <mutex>
#include "DatabaseManager.h"
#include "DiscordManager.h"
#include "AnticheatWebServer.h"
#include <queue>

enum LogLevel {
    L_INFO,     
    L_SUCCESS,  
    L_WARN,     
    L_ERROR,    
    L_DEBUG    
};

struct GUILogEntry {
    std::string timeStr;
    std::string levelStr;
    std::string moduleStr;
    std::string message;
    LogLevel level;
};


extern void GUI_Log(LogLevel level, const std::string& moduleName, const std::string& message);

class She3aSrv
{

public :
    std::string CustomBotToken = "DiscordToken";
    std::string AuthChannel = "1234";
    std::string BanChannel = "1234";
    std::string HeartbeatChannel = "1234";
    std::string WebPanelPassword = "VortexPWz";
    std::string CurrentWebToken = "SHE3A";       
    std::atomic<bool> bEnableAutoBan{ false };
    std::vector<BYTE> GetPlayerLiveFrame(int targetUSN);

    std::mutex ConfigMutex;

private:

    SOCKET ListenSocket;
    SOCKET UdpListenSocket; 

    std::unordered_map<SOCKET, std::shared_ptr<PlayerInfo>> OnlinePlayers;
    std::map<unsigned long, std::string> PendingFileRequests;
    std::mutex PlayersMutex;
    std::unique_ptr<DatabaseManager> db;
    std::unique_ptr<DiscordManager> discord;
    std::unique_ptr<AnticheatWebServer> webServer;

    const int WEB_SERVER_PORT = 8080;
    const int MAX_SERVER_CONNECTIONS = 300; 

    static void ClientThread(std::shared_ptr<PlayerInfo> player);
    void HandlePacket(std::shared_ptr<PlayerInfo> player, BYTE* rawData, int len);
    bool ProcessFinalPacket(std::shared_ptr<PlayerInfo> player, BYTE* rawData, int len);
    void CleanupPlayer(std::shared_ptr<PlayerInfo> player);
    void UpdateNextScreenshotTime(std::shared_ptr<PlayerInfo> player);

    void EncryptDatas(BYTE* datas, size_t len);
    void DecryptDatas(BYTE* datas, size_t len);

    void UdpListenLoop();

    // -- DISCORD WORKER --- //

    void DiscordLogWorker();

public:
    std::string ServerIP = _xor("127.0.0.1").c_str();
    int tServerPort = 1888;
    int uServerPort = 1889;

    std::atomic<int> ConnectedCnt{ 0 };
    std::atomic<int> BannedCnt{ 0 };
    std::atomic<int> DetectedCnt{ 0 };

    static She3aSrv* Instance;

    She3aSrv();
    ~She3aSrv();

    bool Init();
    bool InitServices();
    bool InitDatabase();
    void Run();
    void ReportError(std::string msg);
    static void MonitorWorker();
    bool SendPacket(std::shared_ptr<PlayerInfo> player, BYTE* data, size_t len);

    // --- Config Engine (Encrypted) ---
    void SaveConfig();
    void LoadConfig();


    // --- Helpers ---
    void AddPendingFileRequest(unsigned long usn, const std::string& path) {
        std::lock_guard<std::mutex> lock(PlayersMutex);
        PendingFileRequests[usn] = path;
    }

    // --- WebServer Wrapper ---
    std::string SaveScreenshot(std::shared_ptr<PlayerInfo> player, BYTE* data, size_t len, SCREENSHOT_OPERATION op);

    // --- Discord Wrappers ---
    void LogAuthToDiscord(std::shared_ptr<PlayerInfo> player, const std::string& screenshotUrl = "");
    void LogBanToDiscord(std::shared_ptr<PlayerInfo> player, const char* reason, const char* errorCode, const std::string& screenshotUrl = "");
    void LogHeartbeatSCToDiscord(std::shared_ptr<PlayerInfo> player, const std::string& screenshotUrl);

    // --- Web Panel Actions ---
    std::string GetWebDashboardData();
    void ExecuteWebAction(int targetUSN, const std::string& action, const std::string& reason);

    // --- Database Wrappers ---
    bool DB_ExecuteAuth(std::shared_ptr<PlayerInfo> player);
    bool DB_ExecuteBan(std::shared_ptr<PlayerInfo> player, const char* errorCode, const char* errorMsg, const std::string& proofUrl);
    PLAYER_TYPE DB_GetAuthority(unsigned long usn);
    void AddSuspicionPoints(std::shared_ptr<PlayerInfo> player, int points, const std::string& reason);
    int GetWeightByCode(const std::string& errorCode);
    void DB_UpdateProofUrl(std::shared_ptr<PlayerInfo> player, const std::string& scLink);

    std::string ComputeMD5Hash(const std::string& input);
    const char* GetAuthorityName(PLAYER_TYPE type);


    void Log(LogLevel level, const std::string& moduleName, const std::string& message)
    {
        GUI_Log(level, moduleName, message);
    }
};