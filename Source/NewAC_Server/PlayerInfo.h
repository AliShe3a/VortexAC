/**
 * @file PlayerInfo.h
 * @brief PlayerInfo Class
 */

#ifndef PLAYER_INFO_H
#define PLAYER_INFO_H

#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "../NewAC_Client/xor.h"
#include "../NewAC_Client/xor2.h"
#include "../NewAC_Client/Packets.h"
#include "../NewAC_Client/ErrorCodes.h"
#include <map>
#include <mutex>

struct LiveStreamState {
    bool isActive = false;
    std::vector<unsigned char> latestJPEG; 
    std::mutex frameMutex;

    std::map<unsigned short, std::vector<unsigned char>> currentChunks;
    unsigned long currentFrameID = 0;
};

class PlayerInfo {
public:
    struct {
        int USN = 0;
        int She3aUSN = 0;
        std::string UserName;
        std::string InGameName;
        std::string Password;
        std::string EncryptedPassword;
        std::string ComputerUserName;
        std::string ComputerDomainName;
        std::string UserIP;
        std::string DiscrodID;   
        std::string HardwareUUID;
        std::string HardwareGUID;
        std::string Tokens[10];
        PLAYER_TYPE Authority;
    } Identity;

    struct {

        /// <summary>
        /// Police
        /// </summary>
        std::atomic<bool> isExpectingAuth{ false };
        std::atomic<bool> isExpectingError{ false };
        std::atomic<bool> isExpectingRoomInfo{ false };
        std::atomic<bool> isExpectingHeartbeat{ false };
        std::atomic<bool> isExpectingScreenshot{ false };
        std::atomic<bool> isExpectingDiscord{ false };
        std::atomic<bool> isAwaitingEvidence{ false };
        std::atomic<bool> isAwaitingAuthEvidence{ false };
        std::atomic<bool> isAwaitingHeartbeatEvidence{ false };

        std::atomic<bool> RequestDiscordDataTrigger{ false };
        std::chrono::steady_clock::time_point LastPulseTime;            
        std::chrono::steady_clock::time_point NormalScreenshotTime;      
        std::chrono::steady_clock::time_point BanScreenshotTime;      
        std::chrono::steady_clock::time_point HeartBeatScreenshotTime;     
        std::chrono::steady_clock::time_point NextRandomScreenshotTime; 

        std::atomic<bool> IsAuthorized{ false };
        std::atomic<bool> IsSendingData{ false };
        std::atomic<bool> IsDisconnected{ false };
        std::atomic<bool> IsStopped{ false };

        std::atomic<bool> ScreenshotReq{ false };
        std::atomic<bool> ReceivedScreenshot{ false };
        std::atomic<bool> ReceivedBanScreenshot{ false };
        std::atomic<bool> ReceivedHeartbeatScreenshot{ false };

        std::atomic<bool> DiscordDataReceived{ false };
        std::atomic<bool> HasSuspicionRecord{ false };



        std::atomic<bool> IsVoiceConnected{ false };
        
        LiveStreamState LiveStream;


        std::atomic<int> CurrentVoiceChannel{ 0 };
        std::atomic<int> CurrentTeamID{ 0 };
        std::atomic<int> CurrentRoomID{ 0 };


    } State;




    struct {
        std::atomic<bool> IsBanned{ false };
        std::atomic<bool> IsHwidBanned{ false };
        std::atomic<bool> IsDetected{ false };

        BAN_TYPE BanType = NO_BAN;
        std::string BanReason;

        DETECT_TYPE DetectType = NO_DETECT;
        std::string ErrorCode;
        std::string ErrorMsg;

        std::string DetectName;
        std::string DetectString;

        std::atomic<int> SuspicionScore{ 0 }; 
        std::atomic<int> packetRateCount{ 0 };



        std::vector<BYTE> StreamBuffer;
        std::map<PACKET_ID, std::vector<BYTE>> reassemblyMap;
        std::chrono::steady_clock::time_point lastChunkTime;
        std::chrono::steady_clock::time_point lastRateReset;
        std::chrono::steady_clock::time_point assemblyStartTime;
        std::chrono::steady_clock::time_point evidenceStartTime;
        std::chrono::steady_clock::time_point AuthEvidenceStartTime;
        std::chrono::steady_clock::time_point HeartbeatEvidenceStartTime;

        unsigned long long lastReceivedTimestamp = 0;

    } Security;

    SOCKET hSocket;
    SOCKET UdpListenSocket; 
    sockaddr_in UdpEndpoint;
    std::string LastErrorCode = "0_0";
    std::chrono::steady_clock::time_point ConnectionTime;



    void ResetSession() {
        State.IsAuthorized = false;
        State.IsDisconnected = false;
        //State.IsExpectingData = false;
        State.IsStopped = false;
        Security.IsDetected = false;
        Security.SuspicionScore = 0;
        ConnectionTime = std::chrono::steady_clock::now();
    }

    void PrintStatus() const {
        std::cout << "[Player Status] IGN: " << Identity.InGameName
            << " | USN: " << Identity.USN
            << " | IP: " << Identity.UserIP << std::endl;
        std::cout << " > Auth: " << (State.IsAuthorized ? "YES" : "NO")
            << " | Detected: " << (Security.IsDetected ? "!!! YES !!!" : "NO")
            << " | Suspicion: " << Security.SuspicionScore << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
    }

    PlayerInfo(SOCKET s) {
        hSocket = s;
        auto now = std::chrono::steady_clock::now();
        ConnectionTime = now;
        State.LastPulseTime = now;

        State.NextRandomScreenshotTime = now + std::chrono::minutes(3);
    }

    PlayerInfo() : PlayerInfo(INVALID_SOCKET) {}
};

#endif // PLAYER_INFO_H