#pragma once

const long long acVersion = 0x2;

enum PACKET_ID : unsigned char {
	CS_AUTH_REQ,
	CS_ERROR_REPORT_REQ,
	CS_ROOMINFO_REPORT_REQ,
    CS_ERROR_REPORT,
	CS_ROOMINFO_REPORT,
	CS_SCREENSHOT_DATA,
    CS_HEARTBEAT_MSG,
    CS_LOGOUT_REQ,
	SC_AUTHDATA_REQ,
	SC_HEARTBEAT_REQ,
	SC_SCREENSHOT_REQ,
    SC_APPROVE_ERROR,
    SC_APPROVE_ROOMINFO,
    SC_RESPONSE_AUTHDATA,
    SC_RESPONSE_ERROR_REPORT,
    SC_LIVESHARE_CMD,

};

enum PLAYER_TYPE : unsigned char {
	NORMAL_PLAYER = 0x01,
	GM_PLAYER = 0x02,
	ADMIN_PLAYER = 0x03
};

enum BAN_TYPE : unsigned char {
    NO_BAN,
    NORMAL_BAN,
    HWID_BAN,
};

enum DETECT_TYPE : unsigned char {
    NO_DETECT,
    NORMAL,
    CRITICAL
};

enum SCREENSHOT_OPERATION : unsigned char {
	NORMAL_REQ,
	REPORT_REQ,
	HEARTBEAT_REQ,
    SUSPECTED_REQ
};

enum ROOM_INFO : unsigned char {
    GM_EVENT = 0x01,
    GM_CHECK = 0x02
};

enum GAME_MODES : int
{
    //ZM
    ZA = 9,
    ZA2 = 23,
    ZA3 = 31,

    //Shadow
    Shadow = 16,

    //Common
    TDM = 1,
    SnD = 0,
    WeaponMaster = 35,
    BombingMode = 29,
    RapidSurgeMode = 20,
    GhostMode = 2,
    WipeOut = 6,
    FreeForAll = 3,
    EscapeMode = 8,
    SpyMode = 21,
    CaptainMode = 22,

    //Mutant
    HeroModeX = 12,
    ZombieKnightMode = 18,
    ZombieVsGhost = 17,
    ZombieMode = 4,
    HeroMode = 10,
    MutantChallenge = 47,

    //Wave
    WaveMode = 14,

    //SSD
    SuperSoldierTD = 19,
    SuperSoldierDM = 24,

    //BOT
    AIBotDestruction = 30,
    BotTD = 25,

    //Event
    CopsAndRobbers = 40,

    //GDM
    GDM = 51,

    //BATTLE ROYAL
    BR_SOLO = 41,
    BR_TEAM = 42,
    BR_DUO = 43,


	NONE = -1
};

enum UDP_PACKET_TYPE : unsigned char {
    UDP_TYPE_NONE = 0,
    UDP_TYPE_STREAM = 1,
    UDP_TYPE_VOICE = 2
};

enum VOIP_CHANNEL : unsigned char {
    CH_GLOBAL = 0,
    CH_TEAM = 1
};

#pragma pack(push, 1)
struct PACKET_HEADER {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SEND_AUTH_REQUEST {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    unsigned long USN;
    char LoginID[12];
    char UserIGN[20];
    char UserIP[20];
    char Password[20];
    char UserName[20];
    char ComputerName[20];
    char DiscordID[22];
    char HardwareUUID[50];
    char HardwareGUID[50];
    unsigned long CurVersion;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AUTH_REQUEST_RESPONSE {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned long long Timestamp;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long USN;
    PLAYER_TYPE PlayerType;
    unsigned long Version;
};
#pragma pack(pop)

#pragma pack(push, 1) 
struct SEND_SCREENSHOT {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;  
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    unsigned long USN;
    SCREENSHOT_OPERATION Operation; 
    unsigned int ScreenshotSize;   
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SEND_ERROR_REPORT {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    unsigned long USN;
    char LoginID[12];
    char Password[20];
    char ErrorCode[6];
    char ErrorMsg[1024];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SC_SCREENSHOT_REQUEST {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    SCREENSHOT_OPERATION Operation;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SC_FILEARCHIVE_REQUEST {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    char FilePath[1024];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AC_MSG_PACKET {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    unsigned long USN;
  //  unsigned long Version;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PLAYER_ENTRY {
    char Name[16]; 
    unsigned long TeamID;
    unsigned short KillCount;
};

struct SEND_ROOMINFO_REPORT {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;
    unsigned long USN;
    PLAYER_TYPE PlayerType;
    ROOM_INFO Reason;
    char Reporter[16];
    PLAYER_ENTRY Players[32];   
    unsigned long CashAmount;
	GAME_MODES GameMode;
};


#pragma pack(pop)


// --- NEW: Live Share Enums & Structs ---
enum LIVESHARE_ACTION : unsigned char {
    LS_START,
    LS_STOP,
    LS_UPDATE
};

#pragma pack(push, 1)
struct SC_LIVESHARE_COMMAND {
    unsigned int Len;
    unsigned long CRC;
    PACKET_ID PacketID;
    unsigned short CurCount;
    unsigned short ExtCount;
    unsigned long long Timestamp;

    LIVESHARE_ACTION Action;
    int Quality;   // 1080, 720, 480
    int TargetFPS; // 60, 30, 15
    bool EnableAudio;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct UDP_FRAME_HEADER {
    UDP_PACKET_TYPE Type;    
    unsigned long USN;       
    unsigned long RoomID;   

    VOIP_CHANNEL Channel;    
    unsigned char TeamID;    
    bool IsTalking;          
    char IGN[16];            

    unsigned long FrameID;
    unsigned short ChunkIdx;
    unsigned short MaxChunks;
    unsigned int PayloadLen; 
};
#pragma pack(pop)