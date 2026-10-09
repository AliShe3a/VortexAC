/**
 * @file DatabaseManager.h
 * @brief كلاس إدارة قاعدة البيانات - مخصص للتعامل مع Stored Procedures
 */

#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <mutex>
#include <memory>
#include "../NewAC_Client/Packets.h"

#define MAX_DATABASES 2

class DatabaseManager {
private:
    SQLHENV hEnv;
    SQLHDBC hDbcs[MAX_DATABASES];
    std::mutex dbMutex;           
    bool isConnected;

    void ReportError(SQLSMALLINT handleType, SQLHANDLE handle);

public:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool Connect();
    void Disconnect();

    int GetSavedScore(const char* usn);
    bool UpdateSuspicionScore(const char* guid, int newScore, const char* reason);
    bool UpdateBanProofUrl(unsigned long usn, const char* proofUrl);

    bool ExecuteAuth(unsigned long she3aUSN, const char* loginID, const char* password,
        const char* userName, const char* computerName,
        const char* hwid_uuid, const char* hwid_guid,
        const char* ip);

    bool ExecuteBan(unsigned long targetUSN, const char* errorCode, const char* userId,
        const char* password, int warning, int status,
        const char* proofUrl, const char* errorMsg);
    PLAYER_TYPE GetPlayerAuthority(unsigned long usn);

    bool IsDbConnected() const { return isConnected; }
};