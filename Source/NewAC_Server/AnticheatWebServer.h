/**
 * @file AnticheatWebServer.h
 * @brief تعريف كلاس الويب سيرفر المطور (يدعم الصور والـ Web Dashboard API)
 */

#pragma once
#include <winsock2.h>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include "../NewAC_Client/Packets.h"

class AnticheatWebServer {
private:
    SOCKET listenSocket;
    int port;
    std::string serverIP;
    std::string rootDirectory;
    std::atomic<bool> isRunning;

    std::map<std::string, std::string> mimeTypes;

    void ListenLoop();
    void HandleRequest(SOCKET clientSocket);
    void SendError(SOCKET client, int code, const std::string& msg);

    void SendJSONResponse(SOCKET client, const std::string& json);
    void SendHTMLResponse(SOCKET client, const std::string& html);


public:
    AnticheatWebServer();
    ~AnticheatWebServer();

    AnticheatWebServer(const AnticheatWebServer&) = delete;
    AnticheatWebServer& operator=(const AnticheatWebServer&) = delete;



    bool Start(int port, const std::string& rootDir, const std::string& ip);
    std::string SaveScreenshot(int userUSN, const unsigned char* imageData, size_t dataSize, SCREENSHOT_OPERATION op);
    std::string SaveArchiveFile(int userUSN, const unsigned char* fileData, size_t dataSize);

    std::string ExtractJSONValue(const std::string& json, const std::string& key);

    void Stop();
    bool IsRunning() const { return isRunning; }
};