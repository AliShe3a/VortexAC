/**
 * @file DiscordManager.h
 * @brief تعريف كلاس إدارة الديسكورد - المستوى الاحترافي (يدعم Multi-Tokens)
 */

#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>

namespace DiscordColors {
    const uint32_t Red = 0xFF0000;
    const uint32_t Green = 0x00FF00;
    const uint32_t Blue = 0x000000FF;
    const uint32_t Maroon = 0x800000;
    const uint32_t Gold = 0xFFD700;
}

class DiscordManager {
private:
    std::string masterToken; 
    bool isInitialized;

    std::string EscapeJSON(const std::string& s);

    bool PostToDiscord(const std::string& channelId, const std::string& jsonPayload, const std::string& tokenOverride = "");
public:
    DiscordManager();
    ~DiscordManager();

    DiscordManager(const DiscordManager&) = delete;
    DiscordManager& operator=(const DiscordManager&) = delete;

    bool Init(const std::string& token);

    std::string BuildEmbed(
        const std::string& title,
        uint32_t color,
        const std::vector<std::pair<std::string, std::string>>& fields,
        const std::string& imageUrl = ""
    );

    // تم إضافة tokenOverride هنا أيضاً
    void SendLog(const std::string& channelId, const std::string& payload, const std::string& tokenOverride = "");
    bool PostToWebhook(const std::string& webhookPath, const std::string& jsonPayload);

    void RunGatewaySession();
    bool IsReady() const { return isInitialized && !masterToken.empty(); }
};