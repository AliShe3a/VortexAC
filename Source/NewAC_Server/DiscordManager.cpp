/**
 * @file DiscordManager.cpp
 */
#include "DiscordManager.h"
#include <winhttp.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <thread>
//#include "AntiCheat.h"

#pragma comment(lib, "winhttp.lib")

DiscordManager::DiscordManager() : masterToken(""), isInitialized(false) {}
DiscordManager::~DiscordManager() {}

bool DiscordManager::Init(const std::string& token) {
    if (token.empty()) return false;
    this->masterToken = token;
    this->isInitialized = true;

    std::thread([this]() {
        this->RunGatewaySession();
        }).detach();

    return true;
}

void DiscordManager::RunGatewaySession() {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL, hWebSocket = NULL;

    while (isInitialized) {
        hSession = WinHttpOpen(L"Vortex-Gateway/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) break;

        hConnect = WinHttpConnect(hSession, L"gateway.discord.gg", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); break; }

        hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/?v=10&encoding=json", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); break; }

        unsigned long opt = WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0);

        if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0) && WinHttpReceiveResponse(hRequest, NULL)) {
            hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, NULL);
            if (hWebSocket) {
                std::ostringstream identify;
                identify << "{"
                    << "\"op\": 2,"
                    << "\"d\": {"
                    << "\"token\": \"" << masterToken << "\","
                    << "\"intents\": 0,"
                    << "\"properties\": {\"$os\": \"windows\", \"$browser\": \"Vortex-AC\", \"$device\": \"Vortex-AC\"},"
                    << "\"presence\": {"
                    << "\"activities\": [{\"name\": \"Vortex Server\", \"type\": 3}],"
                    << "\"status\": \"online\","
                    << "\"afk\": false"
                    << "}"
                    << "}}";

                std::string identifyPayload = identify.str();
                WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, (PVOID)identifyPayload.c_str(), (DWORD)identifyPayload.length());

                while (isInitialized) {
                    std::string heartbeat = "{\"op\": 1, \"d\": null}";
                    DWORD res = WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, (PVOID)heartbeat.c_str(), (DWORD)heartbeat.length());

                    if (res != ERROR_SUCCESS) break;

                    for (int i = 0; i < 40 && isInitialized; i++) {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
                WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
                WinHttpCloseHandle(hWebSocket);
            }
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        if (isInitialized) std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

std::string DiscordManager::EscapeJSON(const std::string& s) {
    std::ostringstream oss;
    for (auto c : s) {
        switch (c) {
        case '"': oss << "\\\""; break;
        case '\\': oss << "\\\\"; break;
        case '\b': oss << "\\b"; break;
        case '\f': oss << "\\f"; break;
        case '\n': oss << "\\n"; break;
        case '\r': oss << "\\r"; break;
        case '\t': oss << "\\t"; break;
        default:
            if ('\x00' <= c && c <= '\x1f') {
                oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
            }
            else {
                oss << c;
            }
        }
    }
    return oss.str();
}

bool DiscordManager::PostToDiscord(const std::string& channelId, const std::string& jsonPayload, const std::string& tokenOverride) {
    if (!isInitialized) return false;

    std::string activeToken = tokenOverride.empty() ? masterToken : tokenOverride;
    if (activeToken.empty()) return false;

    bool bResult = false;
    HINTERNET hSession = WinHttpOpen(L"She3a-Vortex-AC/2.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"discord.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {
            std::wstring wsUrl = L"/api/v10/channels/" + std::wstring(channelId.begin(), channelId.end()) + L"/messages";
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wsUrl.c_str(),
                NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

            if (hRequest) {
                std::string authHeader = "Authorization: Bot " + activeToken;
                std::wstring wsAuthHeader(authHeader.begin(), authHeader.end());

                WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/json", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
                WinHttpAddRequestHeaders(hRequest, wsAuthHeader.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);

                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    (LPVOID)jsonPayload.c_str(), (DWORD)jsonPayload.length(),
                    (DWORD)jsonPayload.length(), 0)) {
                    if (WinHttpReceiveResponse(hRequest, NULL)) {
                        bResult = true;
                    }
                }
                WinHttpCloseHandle(hRequest);
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }
    return bResult;
}

std::string DiscordManager::BuildEmbed(const std::string& title, uint32_t color,
    const std::vector<std::pair<std::string, std::string>>& fields,
    const std::string& imageUrl) {
    std::ostringstream json;
    json << "{" << "\"content\": \"\"," << "\"embeds\": [{";
    json << "\"title\": \"" << EscapeJSON(title) << "\",";
    json << "\"color\": " << color << ",";
    json << "\"fields\": [";
    for (size_t i = 0; i < fields.size(); ++i) {
        json << "{"
            << "\"name\": \"" << EscapeJSON(fields[i].first) << "\", "
            << "\"value\": \"```" << EscapeJSON(fields[i].second) << "```\", "
            << "\"inline\": true"
            << "}";
        if (i < fields.size() - 1) json << ",";
    }
    json << "],";
    json << "\"footer\": {"
        << "\"text\": \"She3a Anti-Cheat System | Professional Edition\","
        << "\"icon_url\": \"https://cdn.discordapp.com/avatars/357961207019470851/a7f94baf7f0fc696182ea1588be9479d.png\""
        << "}";
    if (!imageUrl.empty()) {
        json << ",\"image\": {\"url\": \"" << EscapeJSON(imageUrl) << "\"}";
    }
    else {
        json << ",\"image\": {\"url\": \"https://cdn.discordapp.com/attachments/1293488640684720221/1294418810169856041/scdisable.png\"}";
    }
    json << "}]" << "}";
    return json.str();
}

bool DiscordManager::PostToWebhook(const std::string& webhookPath, const std::string& jsonPayload) {
    bool bResult = false;

    HINTERNET hSession = WinHttpOpen(L"She3a-Vortex-AC-Webhook/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        DWORD dwProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwProtocols, sizeof(dwProtocols));
        HINTERNET hConnect = WinHttpConnect(hSession, L"discord.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {
            std::wstring wsUrl(webhookPath.begin(), webhookPath.end());
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wsUrl.c_str(),
                NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

            if (hRequest) {
                WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/json", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);

                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    (LPVOID)jsonPayload.c_str(), (DWORD)jsonPayload.length(),
                    (DWORD)jsonPayload.length(), 0)) {
                    if (WinHttpReceiveResponse(hRequest, NULL)) {
                        DWORD dwStatusCode = 0;
                        DWORD dwSize = sizeof(dwStatusCode);

                        WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX,
                            &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);

                        if (dwStatusCode >= 200 && dwStatusCode < 300) {
                            bResult = true;
                        }
                        else {
                         //   printf("Not Working Ya She3a\n");
                        }
                    }
                }
                WinHttpCloseHandle(hRequest);
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }
    return bResult;
}

void DiscordManager::SendLog(const std::string& channelId, const std::string& payload, const std::string& tokenOverride) {
    if (channelId.empty() || payload.empty()) return;
    PostToDiscord(channelId, payload, tokenOverride);
}