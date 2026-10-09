#pragma once
#define ENABLE_VORTEX_LOG 0

#define _CRT_SECURE_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _HAS_STD_BYTE
#undef _HAS_STD_BYTE
#endif
#define _HAS_STD_BYTE 0

#include <cstddef>
#include <set>

#ifndef WINVER
#define WINVER 0x0601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06000000
#endif
#ifndef WDA_NONE
#define WDA_NONE 0x00000000
#endif
#ifndef WDA_MONITOR
#define WDA_MONITOR 0x00000001
#endif
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#define NTDDI_VERSION  0x06010000

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <sdkddkver.h>
#include <windows.h>

// حل مشكلة الـ min و max اللي بتطلبها GDI+ في C++17
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <algorithm>
namespace Gdiplus {
	using std::min;
	using std::max;
}

#include <string>
#include "xor.h"
#include "xor2.h"

#include "Offsets.h"
#include "Packets.h"
#include "modedhead.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <TlHelp32.h>
#include "CRC.h"
#include <cstdio>
#include "CodeVM/VirtualizerSDK.h"
#include <chrono>
#include <d3d9.h>
#include <webview2.h>
#include <wrl.h>
#include <WebView2EnvironmentOptions.h> 
#include <map>
#include <vector>
#include "LithTech/inc/ltbasedefs.h"
#include "LithTech/inc/ltmatrix.h"
#include "LithTech/inc/ltvector.h"
//#include "LithTech/inc/iltmath.h"
#include "LithTech/inc/ltbasedefs.h"
#include "LithTech/inc/ltbasetypes.h"
#include "LithTech/inc/iltcommon.h"
//#include "LithTech/Game/Shared/AutoMessage.h"
#include "LithTech/inc/ltbasedefs.h"
#include "LithTech/inc/ltmatrix.h"
#include "LithTech/inc/ltvector.h"
//#include "lithtech_sdk/inc/iltmath.h"
//#include "lithtech_sdk/inc/ltrotation.h"
#include "LithTech/inc/ltbasetypes.h"
#include "LithTech/inc/iltcommon.h"
#include "LithTech/inc/ILTMessage.h"
#include "LithTech/inc/ltcodes.h"

#include <Gdiplus.h>
#include "MemoryAddr.h"
#include "Detours/detours.h"
#include <d3dx9.h>
#include <mutex>


#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "VirtualizerSDK32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wsock32.lib")

using namespace Microsoft::WRL;
using namespace std;

struct OpenFileInfo {
	bool isChecked;      // هل فحصنا الهيدر والفوتر ولا لسه؟
	bool isEncrypted;    // هل طلع متشفر فعلاً؟
	uint32_t headerSize; // حجم الهيدر عشان نخصمه من الحساب
	std::string fileName;
};

struct CSLock {
	CRITICAL_SECTION* m_cs;
	CSLock(CRITICAL_SECTION* cs) : m_cs(cs) {
		if (m_cs) EnterCriticalSection(m_cs);
	}
	~CSLock() {
		if (m_cs) LeaveCriticalSection(m_cs);
	}
};

extern CRITICAL_SECTION* g_FileMapCs;
extern std::map<HANDLE, OpenFileInfo>* g_OpenFiles;

struct SYSTEM_CODEINTEGRITY_INFORMATION {
	ULONG Length;
	ULONG CodeIntegrityOptions;
};
typedef BOOL(WINAPI* _CloseHandle)(HANDLE);
typedef BOOL(WINAPI* _CreateDirectoryA)(LPCSTR, LPSECURITY_ATTRIBUTES);
typedef void(__stdcall* _ExitProcess)(DWORD);
typedef DWORD(WINAPI* _GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef HANDLE(WINAPI* _GetCurrentProcess)(VOID);
typedef HANDLE(WINAPI* _OpenProcess)(DWORD, BOOL, DWORD);
typedef int (WINAPI* _MessageBox)(HWND, LPCSTR, LPCSTR, UINT);
typedef BOOL(WINAPI* _VirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef LONG(WINAPI* _NtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef unsigned long(__stdcall* _NtQuerySystemInformation)(unsigned long, void*, unsigned long, unsigned long*);
typedef void (WINAPI* EnterCriticalSection_t) (LPCRITICAL_SECTION lpCriticalSection);
typedef int(__cdecl* MsgBox_t)(int a1, int a2, int a3, uintptr_t msgPtr, int a5);
typedef void(__thiscall* SendChatPacket_t)(void* pThis, const char* szMessage);



//extern "C" {
//	BOOL WINAPI GetWindowDisplayAffinity(HWND hWnd, DWORD* pdwAffinity);
//	BOOL WINAPI SetWindowDisplayAffinity(HWND hWnd, DWORD dwAffinity);
//}

enum GAME_LOBBY : int
{

	LOADING_SCREEN = 0,
	LOGIN_SCREEN = 1,

	// LOBBY
	SERVER_SELECT = 17,
	CHANNEL_SELECT = 3,
	CHANNEL_LOBBY = 8,
	IN_ROOM = 13,
	CLAN_TAB = 26,
	LIVE_GAMES = 27,
	TRADE_TAB = 37,
	TOURMENT = 28,


	// STORAGE
	STORAGE_WEAPONS = 4,
	STORAGE_AGENT = 5,
	STORAGE_ITEM = 6,
	STORAGE_ZOMBIE = 25,
	STORAGE_GIFT = 24,
	STORAGE_RECYCLE = 29,
	STORAGE_TRADE = 38,
	STORAGE_VVIP = 30,

	// ITEM SHOP
	IN_SHOP = 18,

	// BLACK MARCKET
	BLACK_MARCKET = 22,
	BM_STORAGE = 33,
	BM_COUPON = 35,
	BM_CRATE = 36,

	// MILEAGE
	MP_SHOP = 31,
};


class CRoomInfo
{
public:
	int32_t RoomID; //0x0000
	char pad_0004[272]; //0x0004
	int32_t GameID; //0x0114
	char pad_0118[20]; //0x0118
	GAME_MODES GameMode; //0x012C
	char pad_0130[16]; //0x0130
	int32_t WeaponType; //0x0140
	char pad_0144[4]; //0x0144
	float RespawnTime; //0x0148
	int32_t MaxPlayerCount; //0x014C
};//Size=0x0130

class CRoomManager
{
public:
	char _0x0000[12];
	CRoomInfo* RoomInfo; //0x000C
};//Size=0x0010

struct oLTRotation
{
public:
	float Quad[4];
};

struct LTransform
{
public:
	D3DXVECTOR3	m_vPos;
	oLTRotation	m_rRot;
	float		m_fScale;
};

struct CTransform
{
	D3DXVECTOR3 Pos;
	char stack_guard[0x100]{};
};

struct CIntersectInfo
{
	CIntersectInfo()
	{
		m_Point = D3DXVECTOR3(0.f, 0.f, 0.f);
		m_hObject = NULL;
		m_hPoly = DWORD();
		m_SurfaceFlags = NULL;
		m_Unknown = NULL;

		memset(m_Padding, 0, 100);
	}

	D3DXVECTOR3			m_Point;								/* 0x00 - 0x0C */
	char				unknown12[16];
	HOBJECT				m_hObject;								/* 0x1C - 0x20 */
	DWORD				m_hPoly;								/* 0x20 - 0x28 */
	uint32_t			m_SurfaceFlags;							/* 0x28 - 0x2C */
	uint32_t			m_Unknown;								/* 0x2C - 0x30 */
	BYTE				m_Padding[100];
};

struct CIntersectQuery
{
	CIntersectQuery()
	{
		m_From = D3DXVECTOR3(0.f, 0.f, 0.f);
		m_To = D3DXVECTOR3(0.f, 0.f, 0.f);
		m_Plane = D3DXVECTOR3(0.f, 0.f, 0.f);
		m_Flags = NULL;
		m_FilterFn = NULL;
		m_FilterActualIntersectFn = NULL;
		m_PolyFilterFn = NULL;
		m_pUserData = NULL;
		m_pActualIntersectUserData = NULL;

		memset(m_Padding, 0, 100);
	}

	D3DXVECTOR3			m_From;									/* 0x00 - 0x0C */
	D3DXVECTOR3			m_To;									/* 0x0C - 0x18 */
	D3DXVECTOR3			m_Plane;								/* 0x18 - 0x24 */
	int					m_Flags;								/* 0x24 - 0x28 */
	ObjectFilterFnCF		m_FilterFn;								/* 0x28 - 0x2C */
	ObjectFilterFnCF		m_FilterActualIntersectFn;				/* 0x2C - 0x30 */
	PolyFilterFn		m_PolyFilterFn;							/* 0x30 - 0x34 */
	void* m_pUserData;							/* 0x34 - 0x38 */
	void* m_pActualIntersectUserData;				/* 0x38 - 0x3C */
	unsigned char		m_Padding[100];
};

template< typename Function >
Function GetVFunc(PVOID Base, DWORD Index)
{
	PDWORD* VTablePointer = (PDWORD*)Base;
	PDWORD VTableFunctionBase = *VTablePointer;
	DWORD dwAddress = VTableFunctionBase[Index];
	return (Function)(dwAddress);
}

class CLTCommon
{
public:
	LTRESULT CreateMessage(ILTMessage_Write*& pMsg)
	{
		typedef LTRESULT(__thiscall* fnCreateMessage)(void*, ILTMessage_Write*&);
		return GetVFunc<fnCreateMessage>(this, 9)(this, pMsg);
	}

	LTRESULT GetObjectFlags(HOBJECT hObj, ObjFlagType flagType, uint32_t& dwFlags)
	{
		typedef LTRESULT(__thiscall* fnGetObjectFlags)(void*, HOBJECT, ObjFlagType, uint32_t&);
		return GetVFunc<fnGetObjectFlags>(this, 17)(this, hObj, flagType, dwFlags);
	}

	LTRESULT SetObjectFlags(HOBJECT hObj, const ObjFlagType flagType, uint32_t dwFlags, uint32_t dwMask)
	{
		typedef LTRESULT(__thiscall* fnSetObjectFlags)(void*, HOBJECT, const ObjFlagType, uint32_t, uint32_t);
		return GetVFunc<fnSetObjectFlags>(this, 18)(this, hObj, flagType, dwFlags, dwMask);
	}

};

class ILTStream
{
public:
	// Index 1: Release - عشان تنضف الميموري
	void Release() {
		GetVFunc<void(__thiscall*)(void*)>(this, 1)(this);
	}

	// Index 2: Read - القراءة وفك التشفير
	uint32 Read(void* pBuffer, uint32 nLen) {
		return GetVFunc<uint32(__thiscall*)(void*, void*, uint32)>(this, 2)(this, pBuffer, nLen);
	}

	// Index 6: SeekTo - القفز فوق الهيدر
	void SeekTo(uint32 nPos) {
		GetVFunc<void(__thiscall*)(void*, uint32)>(this, 6)(this, nPos);
	}

	// Index 7: GetPos - دي اللي عملت لك المشكلة
	// التعديل: لازم تاخد Pointer عشان تشيل القيمة اللي راجعة
	void GetPos(uint32* nPos) {
		GetVFunc<void(__thiscall*)(void*, uint32*)>(this, 7)(this, nPos);
	}

	// Index 8: GetLen - حجم الملف
	void GetLen(uint32* nLen) {
		GetVFunc<void(__thiscall*)(void*, uint32*)>(this, 8)(this, nLen);
	}
};

class CLTModel
{
public:

	LTRESULT GetSocket2(HLOCALOBJ hObj, const char* pSocketName, HMODELSOCKET& hSocket)
	{
		typedef signed int(__thiscall* GetSocket2Fn)(void*, HLOCALOBJ hObj, const char* pSocketName, HMODELSOCKET& hSocket);
		return GetVFunc<GetSocket2Fn>(this, 6)(this, hObj, pSocketName, hSocket);
	}
	LTRESULT GetSocket(HLOCALOBJ hObj, const char* pSocketName, HMODELSOCKET& hSocket) {
		typedef signed int(__thiscall* GetSocketFn)(void*, HLOCALOBJ hObj, const char* pSocketName, HMODELSOCKET& hSocket);
		return GetVFunc<GetSocketFn>(this, 7)(this, hObj, pSocketName, hSocket);
	}
	LTRESULT GetSocketTransform(HLOCALOBJ hObj, HMODELSOCKET hSocket, LTransform& transform, bool bWorldSpace) {
		typedef signed int(__thiscall* GetSocketTransformFn)(void*, HLOCALOBJ hObj, HMODELSOCKET hSocket, LTransform& transform, bool bWorldSpace);
		return GetVFunc<GetSocketTransformFn>(this, 8)(this, hObj, hSocket, transform, bWorldSpace);
	}
	LTRESULT GetPiece(HLOCALOBJ hObj, const char* pPieceName, HMODELPIECE& hPiece) {
		typedef signed int(__thiscall* GetPieceFn)(void*, HLOCALOBJ hObj, const char* pPieceName, HMODELPIECE& hPiece);
		return GetVFunc<GetPieceFn>(this, 10)(this, hObj, pPieceName, hPiece);
	}
	LTRESULT GetPieceHideStatus(HLOCALOBJ hObj, HMODELPIECE hPiece, bool& bHidden) {
		typedef signed int(__thiscall* GetPieceHideStatusFn)(void*, HLOCALOBJ hObj, HMODELPIECE hPiece, bool& bHidden);
		return GetVFunc<GetPieceHideStatusFn>(this, 11)(this, hObj, hPiece, bHidden);
	}
	LTRESULT SetPieceHideStatus(HLOCALOBJ hObj, HMODELPIECE hPiece, bool bHidden) {
		typedef signed int(__thiscall* SetPieceHideStatusFn)(void*, HLOCALOBJ hObj, HMODELPIECE hPiece, bool bHidden);
		return GetVFunc<SetPieceHideStatusFn>(this, 12)(this, hObj, hPiece, bHidden);
	}
	LTRESULT GetNode(HLOCALOBJ hObj, const char* pNodeName, UINT& hNode) {
		typedef signed int(__thiscall* GetNodeFn)(void*, HLOCALOBJ hObj, const char* pNodeName, UINT& hNode);
		return GetVFunc<GetNodeFn>(this, 13)(this, hObj, pNodeName, hNode);
	}
	LTRESULT GetNodeName(HLOCALOBJ hObj, UINT hNode, char* name, unsigned int maxlen) {
		typedef signed int(__thiscall* GetNodeNameFn)(void*, HLOCALOBJ hObj, UINT hNode, char* name, unsigned int maxlen);
		return GetVFunc<GetNodeNameFn>(this, 14)(this, hObj, hNode, name, maxlen);
	}
	LTRESULT GetNodeTransform(HLOCALOBJ hObj, UINT hNode, CTransform* transform, bool bWorldSpace) {
		typedef signed int(__thiscall* GetNodeTransformFn)(void*, HLOCALOBJ hObj, UINT hNode, CTransform* transform, bool bWorldSpace);
		return GetVFunc<GetNodeTransformFn>(this, 15)(this, hObj, hNode, transform, bWorldSpace);
	}
	LTRESULT GetNextNode(HLOCALOBJ hObj, unsigned int hNode, unsigned int& pNext) {
		typedef signed int(__thiscall* GetNextNodeFn)(void*, HLOCALOBJ hObj, unsigned int hNode, unsigned int& pNext);
		return GetVFunc<GetNextNodeFn>(this, 16)(this, hObj, hNode, pNext);
	}
	LTRESULT GetNumChildren(HLOCALOBJ hObj, unsigned int hNode, unsigned int& NumChildren) {

		typedef signed int(__thiscall* GetNumChildrenFn)(void*, HLOCALOBJ hObj, unsigned int hNode, unsigned int& NumChildren);
		return GetVFunc<GetNumChildrenFn>(this, 18)(this, hObj, hNode, NumChildren);
	}
	LTRESULT GetChild(HLOCALOBJ hObj, unsigned int parent, unsigned int index, unsigned int& child) {
		typedef signed int(__thiscall* GetChildFn)(void*, HLOCALOBJ hObj, unsigned int parent, unsigned int index, unsigned int& child);
		return GetVFunc<GetChildFn>(this, 19)(this, hObj, parent, index, child);
	}
	LTRESULT GetParent(HLOCALOBJ hObj, unsigned int node, unsigned int& parent) {

		typedef signed int(__thiscall* GetParentFn)(void*, HLOCALOBJ hObj, unsigned int node, unsigned int& parent);
		return GetVFunc<GetParentFn>(this, 20)(this, hObj, node, parent);
	}
	LTRESULT GetNumNodes(HLOCALOBJ hObj, unsigned int& num_nodes) {

		typedef signed int(__thiscall* GetNumNodesFn)(void*, HLOCALOBJ hObj, unsigned int& num_nodes);
		return GetVFunc<GetNumNodesFn>(this, 21)(this, hObj, num_nodes);
	}

};

class CLTClient
{
public:

	//29/04
	char pad_0000[0x84]; //0x0000
	bool(__cdecl* IntersectSegment)(const CIntersectQuery&, CIntersectInfo*); //0x0084
	char pad_0090[56]; //0x0090
	LTRESULT(__cdecl* FlipScreen)(uint32_t flags); //0x00CC
	LTRESULT(__cdecl* Start3D)(); //0x00D0
	LTRESULT(__cdecl* RenderCamera)(HLOCALOBJ hCamera, float fFrameTime);  //0x00D4
	char pad_00D4[20]; //0x00D8
	LTRESULT(__cdecl* StartOptimized2D)();//0x00EC
	LTRESULT(__cdecl* EndOptimized2D)(); //0x00F0
	char pad_00F0[8]; //0x00F0
	LTRESULT(__cdecl* End3D)(uint32_t flags);//0x00F8
	char pad_00FC[316]; //0x00FC
	LTRESULT(__cdecl* RunConsoleCommand)(const char*); //0x0238

	CLTCommon* GetLTCommon()
	{
		typedef CLTCommon* (__thiscall* ILTCommonFn)(void*);

		// Call the function normally
		ILTCommonFn fn = GetVFunc<ILTCommonFn>(this, 1);
		CLTCommon* pCommon = fn(this);

		// Print the address
		//printf("GetLTCommon returned address: 0x%p\n", pCommon);

		return pCommon;
	}

	CLTModel* GetLTModel()
	{
		typedef CLTModel* (__thiscall* oGetLTModel)(void*);
		return GetVFunc<oGetLTModel>(this, 4)(this);
	}

	LTRESULT OpenFile(const char* pFileName, ILTStream** ppStream)
	{
		typedef LTRESULT(__thiscall* OpenFileFn)(void*, const char*, ILTStream**);
		return GetVFunc<OpenFileFn>(this, 11)(this, pFileName, ppStream);
	}

	LTRESULT SendToServer(ILTMessage_Read* pMsg, uint32 Flags)
	{
		typedef LTRESULT(__thiscall* SendToServerFn)(void*, ILTMessage_Read*, uint32);
		return GetVFunc<SendToServerFn>(this, 119)(this, pMsg, Flags);
	}

	LTRESULT GetObjectPos(HLOCALOBJ hObj, D3DXVECTOR3* vPos)
	{
		typedef signed int(__thiscall* GetObjectPosFn)(void*, HLOCALOBJ hObj, D3DXVECTOR3* vPos);
		return GetVFunc<GetObjectPosFn>(this, 39)(this, hObj, vPos);
	}
	LTRESULT GetObjectBoxMin(HLOCALOBJ hObj, D3DXVECTOR3& mins)
	{
		typedef uint32_t(__thiscall* GetObjectBoxMinFn)(void*, HLOCALOBJ, D3DXVECTOR3&);
		return GetVFunc<GetObjectBoxMinFn>(this, 140)(this, hObj, mins);
	}
	LTRESULT GetObjectBoxMax(HLOCALOBJ hObj, D3DXVECTOR3& maxs)
	{
		typedef uint32_t(__thiscall* GetObjectBoxMaxFn)(void*, HLOCALOBJ, D3DXVECTOR3&);
		return GetVFunc<GetObjectBoxMaxFn>(this, 141)(this, hObj, maxs);
	}

	//LTRESULT IntersectSegment(CIntersectQuery& iQuery, CIntersectInfo* qInfo)
	//{
	//	typedef LTRESULT(__thiscall* IntersectSegmentFn)(void*, CIntersectQuery&, CIntersectInfo*);
	//	return GetVFunc<IntersectSegmentFn>(this, 33)(this, iQuery, qInfo);
	//}

	//LTRESULT IntersectSegment(CIntersectQuery& iQuery, CIntersectInfo* qInfo)
	//{
	//	using IntersectSegment_t = bool(__cdecl*)(CIntersectQuery&, CIntersectInfo*);
	//	DWORD ptr = ((DWORD)this + 0x84);
	//	return reinterpret_cast<IntersectSegment_t>(ptr)(iQuery, qInfo);
	//}

};

class LTObject
{
public:
	char spacer00[4];
	D3DXVECTOR3 Maxes;
	D3DXVECTOR3 Mins;
	char spacer01[196];
	D3DXVECTOR3 Position;

public:
	FORCEINLINE D3DXVECTOR3 GetMins(void)
	{
		return (this != nullptr) ? Mins : D3DXVECTOR3(0, 0, 0);
	}
	FORCEINLINE D3DXVECTOR3 GetMaxes(void)
	{
		return (this != nullptr) ? Maxes : D3DXVECTOR3(0, 0, 0);
	}
	FORCEINLINE D3DXVECTOR3 GetPos(void)
	{
		return (this != nullptr) ? Position : D3DXVECTOR3(0, 0, 0);
	}

	FORCEINLINE D3DXVECTOR3 GetVelocity(void)
	{
		return *(D3DXVECTOR3*)((DWORD)this + 0xFC);
	}
};

class CBasicPlayerInfo
{
public:
	uint32_t BaseInfoPointer;//0x00
	float MovementSpeed;//0x04
	float MovementWalkRate;//0x08
	float MovementDuckWalkRate;//0x0C
	float MovementSideMoveRate;//0x10
	float MovementFBRunAnimRate;//0x14
	float MovementLRRunAnimRate;//0x18
	float MovementFBWalkAnimRate;//0x1C
	float MovementLRWalkAnimRate;//0x20
	float MovementAccelation;//0x24
	float MovementFriction;//0x28
	float JumpTime;//0x2C
	float JumpVelocity;//0x30
	float JumpLandedWaitTime;//0x34
	float JumpLandedNoJumpTimeRate;//0x38
	float JumpRepeatPenaltyMoveRate;//0x3C
	float JumpRepeatPenaltyHeightRate;//0x40
	float JumpLandedMovePenaltyTimeRate;//0x44
	float JumpLandedMovePenaltyMoveRate;//0x48
	float PVPosDefault;//0x4C
	float PVRotationDefault;//0x50
	float PVModelFOV;//0x54
	float PVModelAspect;//0x58
	char _0x0000[8];
	float PVOnlyMoveFlipTime;//0x64
	float PVOnlyMoveGap;//0x68
	float Unknown_00;//0x6C
	float Unknown_01;//0x70
	float DamagePenaltyTime;//0x74
	float DamagePenaltyMoveRate;//0x78
	float C4PlantTime;//0x7C
	float C4DefuseTime;//0x80
	float MaxCanDefuseDistance;//0x84
	float CharacterHiddenAlpha;//0x88
	float CharacterHiddenWalkAlpha;//0x8C
	float CharacterHiddenRunAlpha;//0x90
	float MovementHiddenRate;//0x94
};

class CCameraBase
{
public:
	char pad_0000[12]; //0x0000
	int32_t Mirror; //0x000C
	char pad_0010[84]; //0x0010
	int32_t CameraMode; //0x0064
	char pad_0068[4]; //0x0068
	D3DXVECTOR3 CameraPos; //0x006C
	char pad_0078[20]; //0x0078


	FORCEINLINE D3DXVECTOR3 GetCameraPos()
	{

		return (this != nullptr) ? CameraPos : D3DXVECTOR3(0, 0, 0);

	}


}; //Size: 0x008C

enum WeaponType
{
	Pistol = 0,
	Shotgun = 1,
	SMG = 2,
	Rifle = 3,
	Sniper = 4,
	MachineGun = 5,
	Grenades = 6,
	Knife = 7,
	C4 = 9,
	RAPPEL = 10
};

enum FireType
{
	SingleFire = 1,
	RepeatFire = 2,
	ShrapnelFire = 4,
	DelayFire = 8,
	AlternateFire = 16
};

class CWeapon
{
public:
	int16_t WeaponID; //0x0000
	int8_t Class; //0x0002
	char pad_0003[11]; //0x0003
	char WeaponName[28]; //0x000E

	int16_t GetWeaponIndex()
	{
		return *(int16_t*)((uintptr_t)this);
	}

	BYTE GetWeaponClass()
	{
		return *(BYTE*)((uintptr_t)this + 0x2);
	}

	FireType& GetFireType()
	{
		return *(FireType*)((uintptr_t)this + 0xF94);
	}

	float& GetWeaponFOV()
	{
		return *(float*)((uintptr_t)this + WEAPON_FOV_OFF);//
	}

	float& ReloadAnimRatio()
	{
		return *(float*)((uintptr_t)this + WEAPON_RELOAD_ANIM_RATIO);//
	}

	float& ChangeWeaponAnimRatio()
	{
		return *(float*)((uintptr_t)this + WEAPON_CHANGE_ANIM_RATIO);//
	}

	int& RepeatFire()
	{
		return *(int*)((uintptr_t)this + WEAPON_REPEAT_FIRE);//
	}

	float* FastKnife1()
	{
		return (float*)((uintptr_t)this + 0xEF0);//
	}

	float* FastKnife2()
	{
		return (float*)((uintptr_t)this + 0xF50);//
	}

	float* FastKnife3()
	{
		return (float*)((uintptr_t)this + 0xF20);//
	}

	float* FastKnife4()
	{
		return (float*)((uintptr_t)this + 0xF80);//
	}

	float* LinkAttackAniRatio()
	{
		return (float*)((uintptr_t)this + 0x42E0);//
	}

	float& Range()
	{
		return *(float*)((uintptr_t)this + WEAPON_RANGE);//
	}

	float* KnockBack()
	{
		return (float*)((uintptr_t)this + WEAPON_KNOCKBACK_1);//
	}

	float* KnockBack2()
	{
		return (float*)((uintptr_t)this + WEAPON_KNOCKBACK_2);//
	}

	float* BulletPosOffset()
	{
		return (float*)((uintptr_t)this + WEAPON_BULLET_POS_OFF_1);//
	}

	float* BulletPosOffset2()
	{
		return (float*)((uintptr_t)this + WEAPON_BULLET_POS_OFF_2);//
	}

	float* BulletPosOffset3()
	{
		return (float*)((uintptr_t)this + WEAPON_BULLET_POS_OFF_3);//
	}

	float* ShotReactPitch()
	{
		return (float*)((uintptr_t)this + WEAPON_SHOT_REACT_PITCH_1);// int32
	}

	float* ShotReactPitch2()
	{
		return (float*)((uintptr_t)this + WEAPON_SHOT_REACT_PITCH_2);//
	}

	float* ShotReactPitch3()
	{
		return (float*)((uintptr_t)this + WEAPON_SHOT_REACT_PITCH_3);//
	}

	float* ShotReactYaw()
	{
		return (float*)((uintptr_t)this + WEAPON_SHOT_REACT_YAW_1);// int32
	}

	float* ShotReactYaw2()
	{
		return (float*)((uintptr_t)this + WEAPON_SHOT_REACT_YAW_2);
	}

	float* ShotReactYaw3()
	{
		return (float*)((uintptr_t)this + WEAPON_SHOT_REACT_YAW_3);
	}

	float* DetailPerturbShot()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_PERTURB_SHOT_1);//
	}

	float* DetailPerturbShot2()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_PERTURB_SHOT_2);//
	}

	float* DetailPerturbShot3()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_PERTURB_SHOT_3);//
	}

	float* DetailPerturbShot4()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_PERTURB_SHOT_4);//
	}

	float* DetailPerturbShot5()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_PERTURB_SHOT_5);//
	}

	float* DetailReactPitchShot()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_PITH_SHOT_1);//
	}

	float* DetailReactPitchShot2()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_PITH_SHOT_2);//
	}

	float* DetailReactPitchShot3()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_PITH_SHOT_3);//
	}

	float* DetailReactPitchShot4()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_PITH_SHOT_4);//
	}

	float* DetailReactPitchShot5()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_PITH_SHOT_5);//
	}

	float* DetailReactYawShot()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_YAW_SHOT_1);//
	}

	float* DetailReactYawShot2()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_YAW_SHOT_2);//
	}

	float* DetailReactYawShot3()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_YAW_SHOT_3);//
	}

	float* DetailReactYawShot4()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_YAW_SHOT_4);//
	}

	float* DetailReactYawShot5()
	{
		return (float*)((uintptr_t)this + WEAPON_DETAIL_REACT_YAW_SHOT_5);//
	}

	float* PertubMin()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MIN_1);//
	}

	float* PertubMin2()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MIN_2);//
	}

	float* PertubMin3()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MIN_3);//
	}

	float* PertubMin4()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MIN_4);//
	}

	float* PertubMin5()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MIN_5);//
	}

	float* PertubMax()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MAX_1);//
	}

	float* PertubMax2()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MAX_2);//
	}

	float* PertubMax3()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MAX_3);//
	}

	float* PertubMax4()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MAX_4);//
	}

	float* PertubMax5()
	{
		return (float*)((uintptr_t)this + WEAPON_PERTURB_MAX_5);//
	}

	float* CrossHairRatioPerRealSize()
	{
		return (float*)((uintptr_t)this + WEAPON_CHAIR_PER_REAL_SIZE);//
	}

	float* AmmoDamage()
	{
		return (float*)((uintptr_t)this + 0xBDC);//
	}

	BYTE SubType()
	{
		return *(BYTE*)((uintptr_t)this + WEAPON_SUBTYPE);//
	}
};

class CCharacterFX
{
public:
	//Att
	char pad_0000[0xA4];
	LTObject* LObject;
	char pad_00A8[0xEE54];
	class CWeapon* CurrentWeapon; // 0xEEFC
	char pad_EF00[0x14];
	int16_t WeaponID; // 0xEF14
	char pad_EF16[0x2A];
	int32_t NanoType; // 0xEF40

	D3DXVECTOR3& ViewAngles()
	{
		return *(D3DXVECTOR3*)((uintptr_t)this + 0xB4);
	}

	bool IsNano()
	{
		return *(bool*)((uintptr_t)this + 0xEF20);
	}

	bool IsSpawnProtected()
	{
		return *(bool*)((uintptr_t)this + 0x210);
	}

	bool IsDead()
	{
		return *(bool*)((DWORD)this + 0x200);
	}

	int ZMHealth()
	{
		//89 81 ? ? ? ? 8B 85 ? ? ? ? 89 81 ? ? ? ? C7 81 ? ? ? ? ? ? ? ? 89 81 ? ? ? ? E9 ? ? ? ? 
		return *(int*)((uintptr_t)this + 0x15604);
	}

	bool IsMutant(bool bTrue = false)
	{
		if (!this)
		{
			return false;
		}

		if (!CurrentWeapon)
		{
			return false;
		}

		if (CurrentWeapon->SubType() == 4)
			return true;


		if (NanoType != 2)
			return true;

		/*if (CurrentWeapon->SubType() == 4)
			return true;*/

		return false;
	}
};

class CPlayer
{
public:
	//Att
	int32_t LocalIndex; //0x0000
	HOBJECT Object;
	uint8_t bClientID; //0x0008
	uint8_t bTeam; //0x0009
	char szName[12]; //0x000A
	char pad_0016[2]; //0x0016
	class CCharacterFX* CharacterFX; //0x0018
	uint8_t bAliveFlag; //0x001C
	char pad_001D[7]; //0x001D
	uint8_t bHasC4; //0x0024
	char pad_0025[27]; //0x0025
	uint16_t Health; //0x0040
	uint16_t Kills;
	uint32_t Deaths;
	uint32_t HeadShots;
	uint32_t TeamID;
	uint32_t Ping;

	inline bool IsValidClient()
	{
		if (this == nullptr)
			return false;

		if (this->Object == nullptr)
			return false;

		if (this->CharacterFX == nullptr)
			return false;

		if (this->bAliveFlag <= 0)
			return false;

		if (this->Health <= 0)
			return false;

		if (strlen(this->szName) <= 0)
			return false;

		return true;
	}

	inline bool IsValidClient2()
	{
		if (this == nullptr)
			return false;

		if (this->Object == nullptr)
			return false;

		if (this->CharacterFX == nullptr)
			return false;

		if (strlen(this->szName) <= 0)
			return false;

		return true;
	}

};

class CPlayerClient
{

public:

	char pad_0000[0x2BC]; //0x0000
	int32_t GunCurrentAmmo; //0x02BC 
	int32_t GetMaxTotalAmmo; //0x02C0 
	int32_t GunClipMaxAmmo; //0x02C4 
	char pad_0x02C8[0x8]; //0x02C8
	class CPlayerViewManager* PlayerViewManager; //0x02D0 
	char pad_0x02D4[0x358]; //0x02D4
	int32_t WeaponID; //0x062C 
	char pad_0x0630[0x308]; //0x0630
	float FakeYaw; //0x0938 
	char pad_0x093C[0x3F0]; //0x093C
	float GameTime; //0x0D2C 
	char pad_0x0D30[0x44]; //0x0D30
	__int32* PlayerObject1;
	__int32* PlayerObject2;
	char pad_0x0D7C[0x8]; //0x0D7C
	int32_t Flags; //0x0D84 
	float flPitch; //0x0D88 
	float flYaw; //0x0D8C 
	//char pad_0x0D90[0x22B0]; //0x0D90
};

typedef char(__thiscall* hGetLocalPlayerIndex)(std::uintptr_t);
extern hGetLocalPlayerIndex GetLocalPlayerIndex;

class CLTClientShell
{
public:
	char pad_0000[48]; //0x0000
	class CCameraBase* Camera; //0x0030
	char pad_0034[48]; //0x0034
	uint8_t bIsAlive; //0x0064
	char pad_0065[19]; //0x0065
	class CPlayerClient* PlayerClient; //0x0078
	uint8_t bInGame; //0x007C
	char pad_007D[15]; //0x007D
	class CLTClient* LTClient; //0x008C
	char pad_0090[52]; //0x0090
	class CCameraInstance* CameraInstance; //0x00C4

	FORCEINLINE class CPlayer* GetPlayerByID(int i)
	{
		if (i < 0 || i > MAX_PLAYERS_IN_ROOM)
			return nullptr;

		return (class CPlayer*)((uintptr_t)this + (i * Player_Size) + Player_Offset);
	}

	FORCEINLINE class CPlayer* GetLocalPlayer(void)
	{
		if (!GetLocalPlayerIndex)
			return nullptr;

		return GetPlayerByID(GetLocalPlayerIndex((uintptr_t)this));
	}

};



class GAME_ENGINE
{
public:
	CLTClient* CLTClient; //0x0000
	char pad_0004[8]; //0x0004
	CLTClientShell* CLTClientShell; //0x000C



};

extern GAME_ENGINE* GameEngine;

struct HookInfo
{
	DWORD address;
	std::string moduleName;
	std::string modulePath;
	DWORD crc;
	bool isSigned;
	std::string publisherName;
	std::string SerialCode;
};


struct OverlayTracker {
	std::chrono::steady_clock::time_point firstSeen;
	bool isActive;
};

struct OwnDllInfo {
	string modulePath;
	string moduleName;
	DWORD crc;
	DWORD moduleBase;
	DWORD moduleSize;
};

struct OverlayFileInfo {
	bool isSigned;
	bool isWhitelisted;
	std::string serialCode;
	std::string publisherName;
	std::string fileName;
	std::string className;
};

struct GameServer {
	char padding[0x3C];  // Reserve space up to offset 0x3C
	int port;            // At offset 0x3C
	uint8_t ip[4];
};

struct VOIPPlayer {
	unsigned long USN;
	std::string IGN;
	bool isTalking;
	DWORD lastPacketTime; // عشان نمسحه لو طلع من الروم
};

struct VoiceChatData {
	bool isConnected = false;
	bool isMicMuted = false;
	bool isSpeakerMuted = false;
	bool isUIVisible = false;

	float micVolume = 1.0f;

	// [NEW] القنوات
	VOIP_CHANNEL currentChannel = CH_GLOBAL;

	// [NEW] قائمة اللاعبين النشطين
	std::map<unsigned long, VOIPPlayer> ActivePlayers;
	std::set<unsigned long> MutedPlayers;
	std::mutex playersMutex;

	HANDLE hCaptureThread = NULL;
	HANDLE hPlaybackThread = NULL;
	SOCKET udpVoiceSocket = INVALID_SOCKET;
	SOCKADDR_IN udpServerAddr;

	// إحداثيات القائمة (بقت أطول شوية عشان تساع اللاعبين)
	int uiX = 0, uiY = 0, uiWidth = 210, uiHeight = 220;
	float animProgress = 0.0f;
	DWORD lastFrameTime = 0;

	// إحداثيات الزراير الجديدة
	int iconX = 0, iconY = 0, iconW = 20, iconH = 50;
	int btnGlobalX = 0, btnTeamX = 0, btnChannelY = 0, btnChannelW = 0, btnChannelH = 25;
	int btnMicX = 0, btnSpkX = 0, btnConnX = 0, btnToolsY = 0, btnToolsW = 0, btnToolsH = 25;

	int sliderX = 0, sliderY = 0, sliderW = 0, sliderH = 0;


};

class PlayerInfo;

class CEngine
{

public:
	SendChatPacket_t oSendChatPacket = nullptr;
	MsgBox_t MsgBox_Fn = nullptr;
	static PlayerInfo* pInstance;
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	HWND GetProcessWindow();

private:

	
	static CEngine* Instance;

	HWND g_GameHWND = NULL;
	const DWORD WEBVIEW_KILL_TIMEOUT = 15000;
	bool g_CaptureEverything = true;




	static CEngine* GetInstance() {
		
		return Instance;
	}

	struct EnumData {
		DWORD dwProcessId;
		HWND hWnd;
	};

	struct WindowAffinityData {
		HWND hwnd;
		DWORD originalAffinity;
	};
	static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);
	static BOOL CALLBACK GlobalEnumWindowsProc(HWND hwnd, LPARAM lParam);
	std::wstring ReadRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName);
	bool WriteRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& value);

	// الـ Hook المحدث
	static int __cdecl MsgBox_hk(int a1, int a2, int a3, uintptr_t msgPtr, int a5) {
		// لو فيه Popup ظهر والـ WebView شغال، نرفع العلمين
		if (GetInstance()->g_IsWebViewInitialized && GetInstance()->g_CurrentVisibleState) {
			GetInstance()->g_ForceHideByPopup = true;
			GetInstance()->g_PopupInterrupted = true;
		}

		// مناداة الفنكشن الأصلية باستخدام الـ Instance المخزن
		return GetInstance()->MsgBox_Fn(a1, a2, a3, msgPtr, a5);
	}

	static void __fastcall hkSendChatPacket(void* pThis, void* _EDX, const char* szMessage);


	void* SafeDetour(BYTE* src, BYTE* dst, const uintptr_t len) {
		if (len < 5) return nullptr;

		// 1. حجز ميموري للـ Gateway (Trampoline)
		BYTE* gateway = (BYTE*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		// 2. نسخ الـ Instructions الأصلية للـ Gateway
		memcpy(gateway, src, len);

		// 3. حساب الـ JMP اللي هيرجع من الـ Gateway للفنكشن الأصلية (بعد الـ Hook)
		uintptr_t gatewayRelativeAddr = (uintptr_t)src - (uintptr_t)gateway - 5;
		*(gateway + len) = 0xE9; // JMP Opcode
		*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddr;

		// 4. تغيير حماية الفنكشن الأصلية عشان نكتب الـ JMP بتاعنا
		DWORD curProtection;
		VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

		// 5. حساب المسافة للـ JMP من الأصلية لـ كودنا (Detour)
		uintptr_t relativeAddr = (uintptr_t)dst - (uintptr_t)src - 5;
		*src = 0xE9;
		*(uintptr_t*)(src + 1) = relativeAddr;

		// 6. مسح أي بايتات زيادة بـ NOP (عشان لو الـ len أكبر من 5)
		for (uintptr_t i = 5; i < len; i++) {
			*(src + i) = 0x90;
		}

		// 7. ترجيع الحماية زي ما كانت
		VirtualProtect(src, len, curProtection, &curProtection);

		return gateway;
	}

public:
	static ComPtr<ICoreWebView2Controller> webviewController;
	static ComPtr<ICoreWebView2> webviewWindow;
	bool g_IsWebViewInitialized = false;
	bool g_IsInitializing = false; // عشان منبعتش طلبات تهيئة كتير ورا بعض
	bool g_CurrentVisibleState = false;
	bool g_ForceHideByPopup = false;      // بيترفع من الـ Hook
	bool g_PopupInterrupted = false; // "القفل" اللي بيمنع الـ Force Open بعد الكراش/البوب أب

	void DisableTopMost();
	std::string GetModuleNameFromAddress(DWORD address);
	int GenerateRandomNumber();
	std::string Clean(const std::string& input);
	char16_t GetCpuID();
	DWORD GetVolumeID();

	template<typename T> bool WPM(int Address, T Value)
	{
		T* address = (T*)(Address);
		*address = Value;
		return true;
	}

	template<typename T> bool vWPM(int address, const T& value, size_t immSize = 4)
	{
		DWORD oldProtect;
		if (!VirtualProtect((LPVOID)(address), immSize, PAGE_EXECUTE_READWRITE, &oldProtect))
			return false;

		bool result = WPM<T>(address, value);

		VirtualProtect((LPVOID)(address), immSize, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), (LPCVOID)(address), immSize);
		return result;
	}

	std::string GetGUID();
	std::string GetUUID();

	DWORD CShell;
	DWORD ObjectDll;
	DWORD CrossFire;
	DWORD ClientFX;
	DWORD pGameDervice = 0;
	DWORD uGameDervice = (0xFBB5B);
	IDirect3DDevice9* g_pd3dDevice = nullptr;


	__declspec(noinline) IDirect3DDevice9* GetDevice();

	void* GetChatInstance()
	{
		typedef void* (__stdcall* GetChatBase_t)(void);
		void* pBase = nullptr;
		DWORD _CallAddr = CShell + INGAME_CHAT_CALL;
		DWORD _EcxAddr = CShell + INGAME_CHAT_ECX;

		__asm {
			push 0x401
			mov ecx, _EcxAddr
			call _CallAddr
			mov pBase, eax
		}

		if (pBase)
			return (void*)((DWORD)pBase + 8); 

		return nullptr;
	}

	void InstallHooks(CEngine* inst) {

		if (!CShell)
			CShell = GetCShellDLL();

		CEngine::Instance = inst;

		if (CShell) {

			MsgBox_Fn = (MsgBox_t)(CShell + 0x19C800);
			oSendChatPacket = (SendChatPacket_t)(CShell + INGAME_CHAT_PACKET);

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			// تبديل الفنكشن الأصلية بالهوك بتاعنا
			DetourAttach(&(PVOID&)MsgBox_Fn, MsgBox_hk);
			DetourAttach(&(PVOID&)oSendChatPacket, hkSendChatPacket);

			DetourTransactionCommit();

			if (MsgBox_Fn && oSendChatPacket) {
				// Hook Success
				printf("MSGBOX & CHATMSG HAS BEEN HOOKED SUCCESSFULLY\n");
			}
		}
	}

	template <typename T>
	T ReadMemSafe(uintptr_t address) {
		if (address == 0) return T();
		try {
			return *reinterpret_cast<T*>(address);
		}
		catch (...) {
			return T();
		}
	}

	template <typename T>
	T RPM(DWORD address) {
		if (address == 0) return T();
		try {
			return *reinterpret_cast<T*>(address);
		}
		catch (...) {
			return T();
		}
	}

	std::string RPMS(DWORD address) {
		if (address == 0) return "";
		try {
			char* strPtr = reinterpret_cast<char*>(address);
			if (IsBadReadPtr(strPtr, 1)) return "";
			return std::string(strPtr);
		}
		catch (...) {
			return "";
		}
	}


	void RunCrashPreventerLogic();


	std::vector<unsigned char> CaptureScreenshot();

	DWORD GetCShellDLL()
	{
		return (DWORD)GetModuleHandleA("cshell.dll");
	}

	DWORD GetObjectDLL()
	{
		return (DWORD)GetModuleHandleA("object.dll");
	}

	CEngine(PlayerInfo* pInfo);

	bool Init();

	void ShowMessage(const char* STRINGEX);
	void ShowNormalMessage(const char* STRINGEX);
	void ChatMessage(const char* msg);
	std::wstring GetGameDir();
	std::string UrlEncode(const std::string& str);
	void KillOrphanedWebViewProcesses();
	void SetGarnetSystemBounds();
	void ShutdownWebView();
	void InternalInitWebView(IDirect3DDevice9* pDevice);
	void RunGarnetLogic(std::wstring& GarLink, bool& GarPopupShowed, uintptr_t& gARAddr, IDirect3DDevice9* pDevice);



};

class PlayerInfo
{

protected:

private:
	std::string GetPComputerName();
	std::string GetPUserName();
	std::string GetDiscordID();
	std::string GetPublicIP();

public:
	static const int TokenMaxCount = 10;

	CEngine* cEngine;
	static PlayerInfo* Instance;
	bool GotUserData = false;

	int UserUSN;
	std::string UserIGN;
	std::string UserURN;
	std::string UserPWD;
	std::string UserIP;
	std::string UserUUID;
	std::string UserGUID;
	std::string UserDID;
	std::string PC_CNAME;
	std::string PC_UNAME;
	std::string AC_SRV_IP;
	std::string BanKey = "NULL";
	std::string BanMsg = "NULL";
	int ZpAmmount = 0;



	PLAYER_TYPE PlayerType = NORMAL_PLAYER;
	bool WantToSendRoomInfo = false;
	bool IsAuthorized = false;
	bool isBannedScReqRecved = false;
	bool isRecvedErrorRptRes = false;

	void GetUserData();
	PlayerInfo()
	{

		printf("[+] Fetching user data...\n");
		Instance = this;
		cEngine = new CEngine(Instance);

		printf("[+] Getting user data from the game and system...\n");

		UserUSN = 0;
	}
	~PlayerInfo()
	{

		UserUSN = 0;
		UserIGN.clear();
		UserURN.clear();
		UserPWD.clear();
		UserUUID.clear();
		UserGUID.clear();
		UserDID.clear();
		PC_CNAME.clear();
		PC_UNAME.clear();


	}
};

class She3aAC
{

public:
	// --- VOICE-CHAT --- //

	VoiceChatData VOIPData;

	void RenderVOIPUI(IDirect3DDevice9* pDevice);
	void HandleVOIPClick(int mouseX, int mouseY);
	void ToggleVOIPConnection();
	static unsigned __stdcall VoiceCaptureWorker(void* lpParam);
	static unsigned __stdcall VoicePlaybackWorker(void* lpParam);

	int GetCurrentGameRoomID();
	int GetCurrentTeamID();
	bool IsTeamMode();

	std::string GetCurrentIGN();

protected:




	BYTE buffer[1024 * 2];
	size_t bufferLen = sizeof(buffer);
	int receivedBytes;

	bool WasFlaggedAsCheater = false;
	bool IsSendingErrorCode = false;
	bool IsWatingForErrorReq = false;
	bool IsSentErrorCode = false;
	bool IsRecvedErrorReportResult = false;
	bool IsSendingData = false;
	bool IsDiscordDataSent = false;
	bool isLiveStreaming = false;


	struct  
	{
	
		int lsQuality = 1280;
		int lsFPS = 30;
		bool lsAudio = false;
		HANDLE hLiveStreamThread = NULL;
	
	} LiveSteamData ;

	HANDLE hCurrentProcess;
	HMODULE D3D9;
	HMODULE Object;
	DWORD ModelNode;
	DWORD* d3dVTable;
	DWORD BasicPlayerInfo;
	DWORD LTClientShell;
	DWORD D3Dx9;
	DWORD Kernel32;
	DWORD ClientFx;
	DWORD FoundDll;
	DWORD ObjectDll;
	_CreateDirectoryA ACCreateDirectory;
	_ExitProcess ACExitProcess;
	_MessageBox ACMessageBox;
	_NtQuerySystemInformation NtQuerySystemInformation;
	_VirtualProtect ACVirtualProtect;
	char D3DVersion[30];
	DWORD D3DModifiedLoop;



private:
	PlayerInfo* cPlayer = NULL;
	uint64_t LocalTimestamp = 0;
	uint64_t ServerTimestamp = 0;


	SEND_ERROR_REPORT acErrorReport = { 0 };
	std::string BanKey = "NULL";
	SOCKET sock = INVALID_SOCKET;
	SOCKET udpLiveSocket = INVALID_SOCKET;
	SOCKADDR_IN sin;
	SOCKADDR_IN udpServerAddr;
	SEND_ROOMINFO_REPORT RoomInfoMessage = {0};

	const char* RmInfoMsg_S = _xor("[VORTEX] Room info sent successfully. Enjoy!").c_str();
	const char* RmInfoMsg_F = _xor("[VORTEX] Error: Failed to send room data.").c_str();

	// --- AC NETWORK -- //
	
	void EncryptDatas(BYTE* datas, size_t len);
	void DecryptDatas(BYTE* datas, size_t len);
	static unsigned __stdcall NetworkListener(void* lpParam);
	bool ConnectToServer();
	bool SendPacketToServer(BYTE* datas, size_t len);
	bool SendAuthRequest();
	bool SendAcHeartbeat();
	bool SendRoomPlayersInfo();
	bool SendHeartbeatResponse();
	bool SaveRoomPlayersInfo(ROOM_INFO Reason);
	bool SendScreenShot(SCREENSHOT_OPERATION Operation);

	//bool RecvDetectionData();



	int PatternCheck;
	int PatternCheck2;
	//	int InGameEngine();
	//	int D3Dinit(void);
	int FirstGame;
	int LoopValue;
	int D3DModifiedLoopCheck;
	int LastGameStatus;
	int hClass;
	bool IsIngameSlowerLoop;
	bool LoopIncrease;
	bool D3DModifiedDone;





	// --- AC DETECTION --- //

	bool IsProcessDetected(); // 10_5
	bool IsXMouseDetected(); // 10_5
	bool IsUMTScriptDetected(); // 10_5
	bool IsIllegalProcessDetected();
	bool IsTestSigningEnabled();
	bool IsTextSectionHookPresent(DWORD AddrStart);
	bool IsHookedCheatCMDDetected();
	bool IsHGWorXignNotLoaded();
	bool AreMultipleClientRunning();
	bool ClientErrorBypassDetected();
	bool IsAntiShakeScreen();
	bool IsBanPacketBypassed();
	bool IsBasicPlayerInfoModified();
	bool IsCHBypassed();
	//	bool IsChatBlocked();
	bool IsD3D9Hooked();
	bool IsD3D9Hooked2();
	bool IsD3DModified();
	bool IsD3DModified2();
	bool IsKernel32ModifiedDetected();
	bool IsKernel32ModifiedDetectedWin7();
	bool IsKernel32ModifiedDetectedWin7_2();
	bool IsEngineHooksModified();
	bool IsInvisibleCharacter();
	bool IsMTPPerfectRecoil();
	bool IsNadeBypassed();
	bool IsNoBugDamageEnabled();
	bool IsNzDBypassed();
	bool IsStringReloadEtcModified();
	bool IsStw();
	bool IsSuperKill();
	bool IsWallArrayModified();
	bool IsGlowHack();
	bool IsRemoteHooked();
	bool IsDllDetected();
	bool IsSendToServerModified();
	void SetAntiWallBan();
	//bool ClientSpeed();
	bool WeaponButesCheckPattern();
	bool ButeFilesModificatioNDetected();
	bool IsBitDefenderDetected();

	// --- HELPER FUNCTIONS --- //

	HANDLE GetProcessHandle(const char* process_name, DWORD dwAccess);
	bool PatternCmp(const BYTE* pData, const BYTE* bMask, const char* szMask);
	bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask);
	//void MsgBoxAddy(DWORD addy);
	DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, const	char* szMask, int chosen = 1);
	DWORD FindPatternVideo(char* module, const char* pattern, const char* mask);
	DWORD FindPatternVideoWindows7(char* module, const char* pattern, const char* mask);
	bool GetCodeSectionInfo(DWORD base_address, DWORD* offset, DWORD* size);
	bool DirectoryExists(const char* dir);
	FARPROC ACGetProcAddress(HMODULE module, const char* proc_name);
	char* GetRegistry(char* Path, std::wstring Key);
	void CustomInfoBox(const char* fmt, ...);

	std::string GetCFServerIP()
	{

		std::string retavlue = _xor("127.0.0.1").c_str();

		if (cPlayer)
			if (cPlayer->cEngine)
				retavlue =  cPlayer->cEngine->RPMS(cPlayer->cEngine->CShell + AC_SERVER_SIP);

	//	retavlue = _xor("127.0.0.1").c_str();
		return retavlue;
    }

	// --- LIVE STREAM --- //
	static unsigned __stdcall LiveStreamWorker(void* lpParam);
	void StopLiveStream();
	void StartLiveStream(int quality, int fps, bool audio);
	// --- AC DETECTION LOOP --- //

	bool IsPlayerInGame();
	void AfterLoginDetections();
	void InGameDetections();
	static unsigned __stdcall HandleAntiCheatDetections();


	// --- ITEM LIMIT --- //

	static unsigned __stdcall GameFlowManager();
	void ItemLimitPatch();
	bool ItemLimitPatch_inGame();
	//bool ItemLimitPatch_inGame(int t);

// --- THREAD MANAGMENT --- //
	static unsigned __stdcall WINAPI GarnetAndProxyWorker(LPVOID lpParam);


	bool ValidateAddress(DWORD Addr, const char* ModuleName);
	bool IsExternalOverlayDetected();
	HookInfo GetHookInfoFromAddress(DWORD AddrStart);
	bool IsHookInfoWhitelisted(const HookInfo& info);
	std::string GetModulePublisherNameSafe(const char* modulePath);
	std::string GetCertSerialCode(const char* filePath);
	bool IsModuleSigned(const char* modulePath);
	OverlayFileInfo GetOverlayFileInfo(const char* processPath, const char* className, const char* windowTitle);
	std::map<HWND, OverlayTracker> m_TrackedOverlays;


	DWORD CalculateCRC32(const BYTE* data, size_t size);
	void InitOwnDllInfo();

	struct ScanContext {
		HWND hGame;
		RECT rGame;
		DWORD pGame;
		She3aAC* pAC;
		bool isFocused;
		bool gameFoundInZOrder;
		bool detected;
		bool isGameActive;
		string ErrorMsg = "NULL";
	};

public:
	static She3aAC* Instance;

	struct ErrorData {
		std::string code;
		std::string msg;
	};

	struct ThreadParams {
		std::string BanKey;
	};


	void SendLogOut();

	static bool SendErrorCode()
	{

		SEND_ERROR_REPORT EmptyStruct = { 0 };
		if (memcmp(&Instance->acErrorReport, &EmptyStruct, sizeof(SEND_ERROR_REPORT)) != 0)
		return Instance->SendPacketToServer((BYTE*)&Instance->acErrorReport, sizeof(SEND_ERROR_REPORT));
		else
			return false;
	}

	bool SendErrorCodeReq(std::string ErrorCode, std::string Message)
	{
		bool success = true;

		AC_MSG_PACKET EmptyStruct = { 0 };
		EmptyStruct.PacketID = CS_ERROR_REPORT_REQ;
		EmptyStruct.USN = Instance->cPlayer->UserUSN + 21;

		memset(&Instance->acErrorReport, 0, sizeof(SEND_ERROR_REPORT));
		Instance->acErrorReport.PacketID = CS_ERROR_REPORT;
		Instance->acErrorReport.USN = Instance->cPlayer->UserUSN + 21;
		strncpy(Instance->acErrorReport.LoginID, Instance->cPlayer->UserURN.c_str(), sizeof(acErrorReport.LoginID) - 1);
		strncpy(Instance->acErrorReport.Password, Instance->cPlayer->UserPWD.c_str(), sizeof(acErrorReport.Password) - 1);
		strncpy(Instance->acErrorReport.ErrorCode, ErrorCode.c_str(), sizeof(acErrorReport.ErrorCode) - 1);
		strncpy(Instance->acErrorReport.ErrorMsg, Message.c_str(), sizeof(acErrorReport.ErrorMsg) - 1);

		Instance->IsWatingForErrorReq = true;

		// إرسال الطلب
		success = Instance->SendPacketToServer(reinterpret_cast<BYTE*>(&EmptyStruct), sizeof(AC_MSG_PACKET));

		Instance->IsWatingForErrorReq = false;
		return success;
	}

	static bool HandleSendErrorCode(ErrorData* data)
	{

		bool sucess = false;

		while (Instance->IsSendingData)
		{
			Sleep(10);
		}
		
		if (data) {
			sucess = Instance->SendErrorCodeReq(data->code, data->msg);
			delete data;
		}


		return sucess;
	}

	She3aAC()
	{

		cPlayer = new PlayerInfo();
		D3D9 = NULL;
		Object = NULL;
		ObjectDll = NULL;
		d3dVTable = NULL;
		WasFlaggedAsCheater = false;
		IsSendingErrorCode = false;
		IsSendingData = false;
		IsDiscordDataSent = false;
		hCurrentProcess = OpenProcess(0x1000, FALSE, GetCurrentProcessId());

	}
	~She3aAC()
	{
		if (hCurrentProcess != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hCurrentProcess);
			hCurrentProcess = INVALID_HANDLE_VALUE;
		}
	}

	static DWORD WINAPI ReportErrorThreadWorker(LPVOID lpParam);
	void ReportError(std::string BanKey);
	bool IsGameDebugged();
	void CloseGame(int uExitCode);
	bool Init();
	void Run();

};

