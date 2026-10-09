/**
 * @file AnticheatWebServer.cpp
 * @brief التنفيذ العملاق لـ C++ Web Engine (يخدم Dashboard والـ API والصور)
 */
#include "AntiCheat.h"
#include "AnticheatWebServer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

AnticheatWebServer::AnticheatWebServer() : listenSocket(INVALID_SOCKET), port(0), isRunning(false) {
    mimeTypes[".html"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".zip"] = "application/zip";
}

AnticheatWebServer::~AnticheatWebServer() { Stop(); }

bool AnticheatWebServer::Start(int port, const std::string& rootDir, const std::string& ip) {
    this->port = port;
    this->rootDirectory = rootDir;
    this->serverIP = ip;
    this->isRunning = true;

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    std::thread(&AnticheatWebServer::ListenLoop, this).detach();
    return true;
}

std::string AnticheatWebServer::SaveScreenshot(int userUSN, const unsigned char* imageData, size_t dataSize, SCREENSHOT_OPERATION op) {
    std::string folderName = "";
    std::string urlPath = "";

    switch (op) {
    case NORMAL_REQ:    folderName = "Screenshots/Auth";      urlPath = "/Screenshots/Auth/"; break;
    case REPORT_REQ:    folderName = "Screenshots/Banned";    urlPath = "/Screenshots/Banned/"; break;
    case HEARTBEAT_REQ: folderName = "Screenshots/Heartbeat"; urlPath = "/Screenshots/Heartbeat/"; break;
    default:            folderName = "Screenshots/Misc";      urlPath = "/Screenshots/Misc/"; break;
    }

    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::string fileName = std::to_string(userUSN) + "_" + std::to_string(ts) + ".jpg";
    std::string fullFilePath = rootDirectory + "/" + folderName + "/" + fileName;

    std::ofstream file(fullFilePath, std::ios::binary);
    if (!file.is_open()) return "";

    file.write((const char*)imageData, dataSize);
    file.close();

    std::ostringstream url;
    url << "http://" << serverIP << ":" << port << urlPath << fileName << "?t=" << ts;
    return url.str();
}

std::string AnticheatWebServer::SaveArchiveFile(int userUSN, const unsigned char* fileData, size_t dataSize) {
    std::string encryptedFolder = "Files";
    std::string decryptedFolder = "Decrypted_Files";
    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::string fileName = "File_" + std::to_string(userUSN) + "_" + std::to_string(ts) + ".zip";

    std::string encryptedPath = rootDirectory + "/" + encryptedFolder + "/" + fileName;
    std::ofstream encFile(encryptedPath, std::ios::binary);
    if (encFile.is_open()) {
        encFile.write((const char*)fileData, dataSize);
        encFile.close();
    }

    std::vector<unsigned char> decryptedData(dataSize);
    const char* secretPass = "She3a";
    size_t passLen = strlen(secretPass);

    for (size_t i = 0; i < dataSize; i++) {
        decryptedData[i] = fileData[i] ^ (unsigned char)secretPass[i % passLen];
    }

    std::string decryptedPath = rootDirectory + "/" + decryptedFolder + "/" + fileName;
    std::ofstream decFile(decryptedPath, std::ios::binary);
    if (decFile.is_open()) {
        decFile.write((const char*)decryptedData.data(), dataSize);
        decFile.close();
        She3aSrv::Instance->Log(L_SUCCESS, "Security", "File Decrypted & Saved: " + fileName);
    }

    std::ostringstream url;
    url << "http://" << serverIP << ":" << port << "/Files/" << fileName << "?t=" << ts;
    return url.str();
}

void AnticheatWebServer::ListenLoop() {
    while (isRunning) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket != INVALID_SOCKET) {
            std::thread(&AnticheatWebServer::HandleRequest, this, clientSocket).detach();
        }
    }
}

std::string AnticheatWebServer::ExtractJSONValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";

    pos += searchKey.length();

    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\"')) pos++;

    size_t endPos = pos;
    while (endPos < json.length() && json[endPos] != '\"' && json[endPos] != ',' && json[endPos] != '}') endPos++;

    return json.substr(pos, endPos - pos);
}

void AnticheatWebServer::SendJSONResponse(SOCKET client, const std::string& json) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << json.length()
        << "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" << json;
    send(client, response.str().c_str(), (int)response.str().length(), 0);
    closesocket(client);
}

void AnticheatWebServer::SendHTMLResponse(SOCKET client, const std::string& html) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << html.length()
        << "\r\nConnection: close\r\n\r\n" << html;
    send(client, response.str().c_str(), (int)response.str().length(), 0);
    closesocket(client);
}

void AnticheatWebServer::HandleRequest(SOCKET client) {
    char buffer[8192];
    int bytesReceived = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) { closesocket(client); return; }

    buffer[bytesReceived] = '\0';
    std::string request(buffer);

    std::istringstream reqStream(request);
    std::string method, path, protocol;
    reqStream >> method >> path >> protocol;

    size_t bodyPos = request.find("\r\n\r\n");
    std::string body = "";

    if (bodyPos != std::string::npos) {
        body = request.substr(bodyPos + 4);

        std::string lowerReq = request;
        for (auto& c : lowerReq) c = tolower(c);

        size_t clPos = lowerReq.find("content-length: ");
        if (clPos != std::string::npos) {
            size_t clEnd = lowerReq.find("\r\n", clPos);
            if (clEnd != std::string::npos) {
                int contentLen = 0;
                try {
                    contentLen = std::stoi(lowerReq.substr(clPos + 16, clEnd - (clPos + 16)));
                }
                catch (...) {}

                while (body.length() < contentLen) {
                    char temp[4096];
                    int r = recv(client, temp, sizeof(temp) - 1, 0);
                    if (r > 0) {
                        temp[r] = '\0';
                        body += temp;
                    }
                    else break;
                }
            }
        }
    }

    std::string queryToken = "";
    std::string queryUsn = "";

    size_t qMark = path.find('?');
    if (qMark != std::string::npos) {
        std::string query = path.substr(qMark + 1);
        path = path.substr(0, qMark);

        size_t tokenPos = query.find("token=");
        if (tokenPos != std::string::npos) {
            queryToken = query.substr(tokenPos + 6);
            size_t ampPos = queryToken.find('&');
            if (ampPos != std::string::npos) {
                queryToken = queryToken.substr(0, ampPos);
            }
        }

        size_t usnPos = query.find("usn=");
        if (usnPos != std::string::npos) {
            queryUsn = query.substr(usnPos + 4);
            size_t ampPos = queryUsn.find('&');
            if (ampPos != std::string::npos) {
                queryUsn = queryUsn.substr(0, ampPos);
            }
        }
    }

    if (method == "GET" && (path == "/" || path == "/index.html")) {
        path = "/website/index.html";
    }
    else if (method == "GET" && path.find("/api/") == std::string::npos &&
        path.find("/Screenshots/") == std::string::npos &&
        path.find("/Files/") == std::string::npos) {
        path = "/website" + path;
    }

    if (method == "POST" && path == "/api/login") {
        if (body == She3aSrv::Instance->WebPanelPassword) {
            She3aSrv::Instance->CurrentWebToken = "VTX-" + std::to_string(GetTickCount());
            SendHTMLResponse(client, "OK:" + She3aSrv::Instance->CurrentWebToken);
        }
        else {
            SendError(client, 401, "Unauthorized");
        }
        return;
    }

    if (path.find("/api/") != std::string::npos && path != "/api/login") {
        std::string reqToken = queryToken.empty() ? ExtractJSONValue(body, "token") : queryToken;
        if (reqToken.empty() || reqToken != She3aSrv::Instance->CurrentWebToken) {
            She3aSrv::Instance->Log(L_DEBUG, "WebServer", "Unauthorized API access blocked! Invalid or missing Token. Path: " + path);
            SendError(client, 401, "Unauthorized");
            return;
        }

        if (method == "GET" && path == "/api/data") {
            SendJSONResponse(client, She3aSrv::Instance->GetWebDashboardData());
            return;
        }

        if (method == "GET" && path == "/api/settings") {
            std::ostringstream json;
            json << "{\"customToken\":\"" << She3aSrv::Instance->CustomBotToken << "\",";
            json << "\"authCh\":\"" << She3aSrv::Instance->AuthChannel << "\",";
            json << "\"banCh\":\"" << She3aSrv::Instance->BanChannel << "\",";
            json << "\"hbCh\":\"" << She3aSrv::Instance->HeartbeatChannel << "\",";
            json << "\"serverIp\":\"" << She3aSrv::Instance->ServerIP << "\",";
            json << "\"autoBan\":" << (She3aSrv::Instance->bEnableAutoBan.load() ? "true" : "false") << "}";
            SendJSONResponse(client, json.str());
            return;
        }

        if (method == "POST" && path == "/api/settings") {
            {
                std::lock_guard<std::mutex> lock(She3aSrv::Instance->ConfigMutex);
                She3aSrv::Instance->CustomBotToken = ExtractJSONValue(body, "customToken");
                She3aSrv::Instance->AuthChannel = ExtractJSONValue(body, "authCh");
                She3aSrv::Instance->BanChannel = ExtractJSONValue(body, "banCh");
                She3aSrv::Instance->HeartbeatChannel = ExtractJSONValue(body, "hbCh");
                She3aSrv::Instance->ServerIP = ExtractJSONValue(body, "serverIp");
                She3aSrv::Instance->bEnableAutoBan = (ExtractJSONValue(body, "autoBan") == "true");
            }

            She3aSrv::Instance->SaveConfig();

            She3aSrv::Instance->Log(L_SUCCESS, "WebPanel", "System configuration updated and encrypted to disk.");
            SendJSONResponse(client, "{\"status\":\"success\"}");
            return;
        }

        if (method == "POST" && path == "/api/action") {
            std::string usnStr = ExtractJSONValue(body, "usn");
            std::string action = ExtractJSONValue(body, "action");
            std::string reason = ExtractJSONValue(body, "reason");

            if (!usnStr.empty()) {
                try {
                    int targetUSN = std::stoi(usnStr);
                    She3aSrv::Instance->ExecuteWebAction(targetUSN, action, reason);
                }
                catch (const std::exception& e) {
                    She3aSrv::Instance->Log(L_ERROR, "WebServer", "Invalid USN format received! Prevented crash.");
                }
            }
            SendJSONResponse(client, "{\"status\":\"success\"}");
            return;
        }

        if (method == "GET" && path == "/api/stream") {
            std::string usnStr = queryUsn;

            She3aSrv::Instance->Log(L_DEBUG, "WebServer", "Browser requested stream for USN: [" + usnStr + "]");

            if (!usnStr.empty()) {
                try {
                    int targetUSN = std::stoi(usnStr);
                    std::vector<BYTE> frame = She3aSrv::Instance->GetPlayerLiveFrame(targetUSN);

                    if (!frame.empty()) {
                        She3aSrv::Instance->Log(L_DEBUG, "WebServer", "Frame ready for USN " + usnStr + ". Size: " + std::to_string(frame.size()) + " bytes. Sending...");

                        std::ostringstream response;
                        response << "HTTP/1.1 200 OK\r\n"
                            << "Content-Type: image/jpeg\r\n"
                            << "Content-Length: " << frame.size() << "\r\n"
                            << "Access-Control-Allow-Origin: *\r\n"
                            << "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                            << "Connection: close\r\n\r\n";
                        send(client, response.str().c_str(), (int)response.str().length(), 0);
                        send(client, (const char*)frame.data(), (int)frame.size(), 0);
                        closesocket(client);
                        return;
                    }
                    else {
                        She3aSrv::Instance->Log(L_DEBUG, "WebServer", "Frame requested but BUFFER IS EMPTY for USN: " + usnStr);
                    }
                }
                catch (...) {
                    She3aSrv::Instance->Log(L_ERROR, "WebServer", "Failed to parse USN string to int: " + usnStr);
                }
            }
            SendError(client, 404, "Frame Not Ready");
            return;
        }

    }

    std::string fullPath = rootDirectory + path;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) { SendError(client, 404, "Not Found"); return; }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> fileBuffer(fileSize);
    file.read(fileBuffer.data(), fileSize);
    file.close();

    std::string contentType = "application/octet-stream";
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);
        if (mimeTypes.find(ext) != mimeTypes.end()) {
            contentType = mimeTypes[ext];
        }
    }

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: " << contentType << "\r\n"
        << "Content-Length: " << fileSize << "\r\n"
        << "Cache-Control: no-cache, no-store, must-revalidate\r\n"
        << "Pragma: no-cache\r\n"
        << "Expires: 0\r\n"
        << "Connection: close\r\n\r\n";

    send(client, response.str().c_str(), (int)response.str().length(), 0);
    send(client, fileBuffer.data(), (int)fileSize, 0);
    closesocket(client);
}

void AnticheatWebServer::SendError(SOCKET client, int code, const std::string& msg) {
    std::ostringstream response;
    response << "HTTP/1.1 " << code << " " << msg << "\r\nContent-Length: 0\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
    send(client, response.str().c_str(), (int)response.str().length(), 0);
    closesocket(client);
}

void AnticheatWebServer::Stop() {
    isRunning = false;
    if (listenSocket != INVALID_SOCKET) { closesocket(listenSocket); listenSocket = INVALID_SOCKET; }
}