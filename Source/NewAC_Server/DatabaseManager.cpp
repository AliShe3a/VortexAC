#include "AntiCheat.h"
#include "DatabaseManager.h"
#include <iostream>
#include <vector>
#include "../NewAC_Client/ErrorCodes.h"

DatabaseManager::DatabaseManager() : hEnv(SQL_NULL_HENV), isConnected(false) {
    for (int i = 0; i < MAX_DATABASES; i++) {
        hDbcs[i] = SQL_NULL_HDBC;
    }
}

DatabaseManager::~DatabaseManager() {
    Disconnect();
}

/**
 * @brief معالجة وإظهار أخطاء SQL بشكل تفصيلي (Native C++)
 */
void DatabaseManager::ReportError(SQLSMALLINT handleType, SQLHANDLE handle) {
    SQLCHAR sqlState[6];
    SQLINTEGER nativeError;
    SQLCHAR messageText[1024];
    SQLSMALLINT textLength;

    if (SQLGetDiagRecA(handleType, handle, 1, sqlState, &nativeError, messageText, sizeof(messageText), &textLength) == SQL_SUCCESS) {
        std::string errMsg = "State: " + std::string((char*)sqlState) + " | Msg: " + std::string((char*)messageText);

        She3aSrv::Instance->Log(L_ERROR, "Database", errMsg);
    }
}

/**
 * @brief إنشاء الاتصال بقواعد البيانات (تحسين لكودك القديم)
 */
bool DatabaseManager::Connect() {
    std::lock_guard<std::mutex> lock(dbMutex);

    // 1. تخصيص البيئة (Environment)
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return false;
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    const char* ipAddress = "103.78.0.191";
    int port = 1433;
    const char* username = "anticheat";
    const char* password = "PassworkAnti5234bX";
    const char* databases[MAX_DATABASES] = { "CF_ANTICHEAT", "CF_SA_GAME" };

    int successfulConnections = 0;

    for (int i = 0; i < MAX_DATABASES; i++) {
        if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbcs[i]) != SQL_SUCCESS) continue;

        char connStr[1024];
        snprintf(connStr, sizeof(connStr),
            "DRIVER={SQL Server};SERVER=%s,%d;DATABASE=%s;UID=%s;PWD=%s",
            ipAddress, port, databases[i], username, password);

        SQLRETURN ret = SQLDriverConnectA(hDbcs[i], NULL, (SQLCHAR*)connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            successfulConnections++;
            She3aSrv::Instance->Log(L_DEBUG, "Database", "Connected to database: " + std::string(databases[i]));
        }
        else {
            ReportError(SQL_HANDLE_DBC, hDbcs[i]);
            SQLFreeHandle(SQL_HANDLE_DBC, hDbcs[i]);
            hDbcs[i] = SQL_NULL_HDBC;
        }
    }

    isConnected = (successfulConnections > 0);
    return isConnected;
}

void DatabaseManager::Disconnect() {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (int i = 0; i < MAX_DATABASES; i++) {
        if (hDbcs[i] != SQL_NULL_HDBC) {
            SQLDisconnect(hDbcs[i]);
            SQLFreeHandle(SQL_HANDLE_DBC, hDbcs[i]);
            hDbcs[i] = SQL_NULL_HDBC;
        }
    }
    if (hEnv != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        hEnv = SQL_NULL_HENV;
    }
    isConnected = false;
}

/**
 * @brief تنفيذ الـ Stored Procedure الخاصة بالـ Auth
 */
bool DatabaseManager::ExecuteAuth(unsigned long she3aUSN, const char* loginID, const char* password,
    const char* userName, const char* computerName,
    const char* hwid_uuid, const char* hwid_guid,
    const char* ip) {

    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected || hDbcs[0] == SQL_NULL_HDBC) return false;

    SQLHSTMT hstmt;
    SQLRETURN ret;

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char formattedTime[42];
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbcs[0], &hstmt);
    if (ret != SQL_SUCCESS) {
        ReportError(SQL_HANDLE_DBC, hDbcs[0]);
        return false;
    }

    const char* functionCall = "{CALL ANTI_CHEAT_AUTH_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?)}";

    long tempUSN = (long)she3aUSN; 

    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &tempUSN, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)loginID, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)password, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 256, 0, (SQLPOINTER)userName, 0, NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 256, 0, (SQLPOINTER)computerName, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, (SQLPOINTER)hwid_uuid, 0, NULL);
    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, (SQLPOINTER)hwid_guid, 0, NULL);
    SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 42, 0, (SQLPOINTER)formattedTime, 0, NULL);
    SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, (SQLPOINTER)ip, 0, NULL);

    ret = SQLExecDirectA(hstmt, (SQLCHAR*)functionCall, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        ReportError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return true;
}

/**
 * @brief تنفيذ الـ Stored Procedure الجديدة بالكامل
 */
bool DatabaseManager::ExecuteBan(unsigned long targetUSN, const char* errorCode, const char* userId,
    const char* password, int warning, int status,
    const char* proofUrl, const char* errorMsg) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected || hDbcs[0] == SQL_NULL_HDBC) return false;

    SQLHSTMT hstmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbcs[0], &hstmt) != SQL_SUCCESS) return false;

    const char* call = "{CALL ANTICHEAT_BANUSER(?, ?, ?, ?, ?, ?, ?, ?)}";
    long tUSN = (long)targetUSN;

    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &tUSN, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)errorCode, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)userId, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)password, 0, NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &warning, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &status, 0, NULL);
    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, (SQLPOINTER)proofUrl, 0, NULL);
    SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 4096, 0, (SQLPOINTER)errorMsg, 0, NULL);

    SQLRETURN ret = SQLExecDirectA(hstmt, (SQLCHAR*)call, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        ReportError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return true;
}

/**
 * @brief جلب رتبة اللاعب من قاعدة البيانات الثانية (CF_SA_GAME / CF_USER)
 */
PLAYER_TYPE DatabaseManager::GetPlayerAuthority(unsigned long usn) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected || hDbcs[1] == SQL_NULL_HDBC) return NORMAL_PLAYER;

    SQLHSTMT hstmt;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbcs[1], &hstmt);
    if (ret != SQL_SUCCESS) {
        ReportError(SQL_HANDLE_DBC, hDbcs[1]);
        return NORMAL_PLAYER;
    }

    const char* query = "SELECT AUTHORITY FROM CF_USER WHERE USN = ?";

    ret = SQLPrepareA(hstmt, (SQLCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        ReportError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return NORMAL_PLAYER;
    }

    long tempUSN = (long)usn;
    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &tempUSN, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        ReportError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return NORMAL_PLAYER;
    }

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        ReportError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return NORMAL_PLAYER;
    }

    char authority[2] = "";
    SQLLEN indicator;

    ret = SQLFetch(hstmt);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLGetData(hstmt, 1, SQL_C_CHAR, authority, sizeof(authority), &indicator);
    }
    else {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return NORMAL_PLAYER;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (strcmp(authority, "A") == 0) return ADMIN_PLAYER;
    if (strcmp(authority, "G") == 0) return GM_PLAYER;
    if (strcmp(authority, "N") == 0) return NORMAL_PLAYER;

    return NORMAL_PLAYER;
}

/**
 * @brief جلب سكور الشك الحالي للاعب من جدول ANTICHEAT_SUSPICION_LOG
 */
int DatabaseManager::GetSavedScore(const char* guid) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected || hDbcs[0] == SQL_NULL_HDBC || !guid) return 0;

    SQLHSTMT hstmt;
    int score = 0;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbcs[0], &hstmt) == SQL_SUCCESS) {
        const char* query = "SELECT CurrentScore FROM ANTICHEAT_SUSPICION_LOG WHERE HardwareGUID = ?";

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)guid, 0, NULL);

        if (SQLExecDirectA(hstmt, (SQLCHAR*)query, SQL_NTS) == SQL_SUCCESS) {
            if (SQLFetch(hstmt) == SQL_SUCCESS) {
                SQLGetData(hstmt, 1, SQL_C_LONG, &score, sizeof(score), NULL);
            }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    return score;
}

/**
 * @brief تحديث سكور الشك في جدول ANTICHEAT_SUSPICION_LOG بناءً على الـ HardwareGUID
 */
bool DatabaseManager::UpdateSuspicionScore(const char* guid, int newScore, const char* reason) {
    std::lock_guard<std::mutex> lock(dbMutex);

    if (!isConnected || hDbcs[0] == SQL_NULL_HDBC || !guid) return false;

    SQLHSTMT hstmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbcs[0], &hstmt) != SQL_SUCCESS) return false;

    const char* query =
        "IF EXISTS (SELECT 1 FROM ANTICHEAT_SUSPICION_LOG WHERE HardwareGUID = ?) "
        "UPDATE ANTICHEAT_SUSPICION_LOG SET CurrentScore = ?, LastUpdate = GETDATE(), TotalViolations = ISNULL(TotalViolations, '') + ';' + ? WHERE HardwareGUID = ? "
        "ELSE "
        "INSERT INTO ANTICHEAT_SUSPICION_LOG (HardwareGUID, CurrentScore, LastUpdate, TotalViolations) VALUES (?, ?, GETDATE(), ?)";


    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)guid, 0, NULL);

    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &newScore, 0, NULL);

    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)reason, 0, NULL);

    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)guid, 0, NULL);

    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)guid, 0, NULL);

    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &newScore, 0, NULL);

    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)reason, 0, NULL);

    SQLRETURN ret = SQLExecDirectA(hstmt, (SQLCHAR*)query, SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        ReportError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return true;
}
/**
 * @brief تحديث رابط الصورة في جدول ANTICHEAT_BAN_LOG بعد استلامها
 */
bool DatabaseManager::UpdateBanProofUrl(unsigned long usn, const char* proofUrl) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected || hDbcs[0] == SQL_NULL_HDBC) return false;

    SQLHSTMT hstmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbcs[0], &hstmt) != SQL_SUCCESS) return false;

    long tUSN = (long)usn;

    const char* query =
        "UPDATE ANTICHEAT_BAN_LOG SET PROOF_URL = ? "
        "WHERE USN = ? AND (PROOF_URL = 'Awaiting Evidence...' OR PROOF_URL IS NULL)";

    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, (SQLPOINTER)proofUrl, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &tUSN, 0, NULL);

    SQLRETURN ret = SQLExecDirectA(hstmt, (SQLCHAR*)query, SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}