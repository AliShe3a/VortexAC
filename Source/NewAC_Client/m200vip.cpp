
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "SAC.h"
#include "VortexHooks.h"



DWORD hCShellBase = 0;
int inGame = 0;
int lastround = 0;
RECT srcRect = { 181, 85, 181 + 442, 85 + 442 };
bool bDrawInit = false;

static DWORD dwHitRunStartTime = 0;
static DWORD dwCooldownStartTime = 0;
static int lastKillCount = -1;     
bool bIsHitRunActive = false;
bool bIsHitRunEnded = false;
bool bIsMemoryApplied = false;   
bool bIsInCooldown = false;
bool bIsMoving = false;
bool bPendingActivation = false;
bool bIsTargetingEffectActive = false;

IDirect3DTexture9* pTexture;
D3DSURFACE_DESC tDesc;
IDirect3DTexture9* pElectronicTexture = nullptr;
hGetLocalPlayerIndex GetLocalPlayerIndex = NULL;
GAME_ENGINE* GameEngine = nullptr;
DWORD FnGetLocalPlayerIndex = 0;

float OriginalMovementSpeed = 0.0f;
float OriginalMovementWalkRate = 0.0f;
float OriginalMovementFBRunAnimRate = 0.0f;
float OriginalDamageRatio = 0.0f;

static DWORD lastScoreboardTick = 0;
static bool bScoreboardState = false; 

const char* TARGET_BONES[] = {
	"M-bone Head",      
	"M-bone Spine1",     
	"M-bone Pelvis",    
	"M-bone R UpperArm", 
	"M-bone L UpperArm", 
	"M-bone R Calf",     
	"M-bone L Calf"    
};
// عدد العظام
const int NUM_TARGET_BONES = 7;

float GetDistance(D3DXVECTOR2 p1, D3DXVECTOR2 p2) {
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

bool GetBonePosition(LTObject* hObject, const char* szBoneName, D3DXVECTOR3* out, bool bWorldSpace)
{
	if (!szBoneName || !hObject || !out)
		return false;

	HMODELNODE hBoneNode = INVALID_MODEL_NODE;

	// Use the same initialization style for transform
	CTransform transform{ D3DXVECTOR3(0, 0, 0) };

	if (GameEngine->CLTClient->GetLTModel()->GetNode(hObject, szBoneName, hBoneNode) == LT_OK)
	{
		// Initialize transform directly in the call to GetNodeTransform
		if (GameEngine->CLTClient->GetLTModel()->GetNodeTransform(hObject, hBoneNode, &transform, bWorldSpace) == LT_OK)
		{
			out->x = transform.Pos.x;
			out->y = transform.Pos.y;
			out->z = transform.Pos.z;

			return true;
		}
	}

	return false;
}


bool TraceFilter(HOBJECT Object, void* Data)
{
	//printf("Entering TraceFilter function\n");

	CPlayer* pLocalPlayer = GameEngine->CLTClientShell->GetLocalPlayer();
	if (!pLocalPlayer)
	{
		//printf("Local player is NULL, returning false\n");
		return false;
	}

	if (Object == pLocalPlayer->Object)
	{
		//printf("Object is the local player, returning false\n");
		return false;
	}

	uint32 Flags;
	CLTCommon* pCommon = GameEngine->CLTClient->GetLTCommon();
	if (!pCommon)
	{
		//printf("pCommon is NULL, returning false\n");
		return false;
	}

	if (pCommon->GetObjectFlags(Object, OFT_Flags, Flags) != LT_OK)
	{
		//printf("Failed to get object flags, returning false\n");
		return false;
	}

	if (!(Flags & FLAG_VISIBLE_CF))
	{
		//printf("Object is not visible (missing FLAG_VISIBLE), returning false\n");
		return false;
	}
	if (!(Flags & FLAG_RAYHIT_CF))
	{
		//printf("Object does not hit the ray (missing FLAG_RAYHIT), returning false\n");
		return false;
	}

	//printf("Object passed the filter, returning true\n");
	return true;
}

inline bool ASM_Intersect(unsigned long TraceFunction, CIntersectQuery* Query, CIntersectInfo* Result)
{
	_asm push	Result
	_asm push	Query
	_asm mov	eax, TraceFunction
	_asm call	eax
	_asm add	esp, 0x8
}

bool IsVisible(HOBJECT hObject, D3DXVECTOR3 Start, D3DXVECTOR3 End)
{
	//printf("Entering IsVisible function\n");

	if (!hCShellBase || !hObject)
	{
		//printf("Offsets::CShell or hObject is NULL\n");
		return false;
	}

	if (!GameEngine->CLTClientShell || !GameEngine->CLTClient)
	{
		//printf("GameEngine->CLTClientShell or GameEngine->CLTClient is NULL\n");
		return false;
	}

	//printf("Setting up intersect query...\n");
	CIntersectQuery iQuery;
	CIntersectInfo iInfo;

	iQuery.m_From = Start;
	iQuery.m_To = End;
	iQuery.m_Flags = INTERSECT_HPOLY_CF | INTERSECT_OBJECTS_CF | IGNORE_NONSOLID_CF;
	iQuery.m_FilterActualIntersectFn = TraceFilter;
	iQuery.m_pActualIntersectUserData = &iQuery;

	if (ASM_Intersect((DWORD)GameEngine->CLTClient->IntersectSegment, &iQuery, &iInfo))
	{
		if (iInfo.m_hObject == hObject)
		{
			//printf("Returning true, object is visible\n");
			return true;
		}
	}

	//printf("Returning false, object is not visible\n");
	return false;
}

bool IsVisible(HOBJECT Obj, D3DXVECTOR3& vTo)
{
	return IsVisible(Obj, GameEngine->CLTClientShell->Camera->GetCameraPos(), vTo);
}

bool IsAnyBoneVisible(LTObject* pObject, IDirect3DDevice9* pDevice)
{
	if (!pObject) return false;

	for (int i = 0; i < NUM_TARGET_BONES; ++i)
	{
		D3DXVECTOR3 bonePos;
		if (GetBonePosition(pObject, TARGET_BONES[i], &bonePos, true))
		{
			if (IsVisible(pObject, bonePos))
			{
				return true;
			}
		}
	}

	return false; 
}

bool WorldToScreen(D3DXVECTOR3 vWorld, D3DXVECTOR3& vScreen, IDirect3DDevice9* pDevice)
{
	if (!pDevice) return false;

	D3DXMATRIX view, projection, world;

	if (FAILED(pDevice->GetTransform(D3DTS_VIEW, &view)))
		return false;

	if (FAILED(pDevice->GetTransform(D3DTS_PROJECTION, &projection)))
		return false;

	D3DXMatrixIdentity(&world);

	D3DVIEWPORT9 vp;
	if (FAILED(pDevice->GetViewport(&vp)))
		return false;

	if (FAILED(D3DXVec3Project(&vScreen, &vWorld, &vp, &projection, &view, &world)))
		return false;

	if (vScreen.z < 0.0f || vScreen.z > 1.0f)
		return false;

	return true;
}

bool IsMutantRoom()
{
	static DWORD dwRoomInfo = NULL;
	if (dwRoomInfo == NULL)
	{
		dwRoomInfo = ((DWORD)(GetModuleHandleA("CShell.dll")) + 0x1668228);
	}

	if (dwRoomInfo != NULL)
	{
		auto RoomManagerAddy = *reinterpret_cast<uintptr_t*>(dwRoomInfo);

		CRoomManager* room = reinterpret_cast<CRoomManager*>(RoomManagerAddy);
		CRoomInfo* Info = room->RoomInfo;

		if (Info)
			return (Info->GameMode == HeroModeX || Info->GameMode == ZombieKnightMode || Info->GameMode == ZombieVsGhost || Info->GameMode == ZombieMode || Info->GameMode == HeroMode || Info->GameMode == MutantChallenge);
	}

	return false;
}

bool IsSameTeam(CPlayer* you, CPlayer* enemy)
{
	bool same_team = (you->bTeam == enemy->bTeam);

	if (IsMutantRoom())
	{

		//std::cout << "Local Player isMutant : " << you->CharacterFX->IsMutant() << std::endl;
		//std::cout << "Enemy Player isMutant : " << enemy->CharacterFX->IsMutant() << std::endl;


		same_team = (you->CharacterFX->IsMutant(true) == enemy->CharacterFX->IsMutant());

		/*if ((you->CharacterFX->IsMutant() != enemy->CharacterFX->IsMutant()))
			same_team = false;
		else
			same_team = true;*/
	}

	return same_team;
}

struct box_data
{
public:
	int x, y, w, h, centerX, centerY;
};


bool calculate_dynamic_box(LTObject* entity, box_data& box, IDirect3DDevice9* pDevice)
{
	if (!entity || !pDevice) return false;

	// دول بييجوا جاهزين (World Coordinates)
	D3DXVECTOR3 min = entity->GetMins();
	D3DXVECTOR3 max = entity->GetMaxes();

	D3DXVECTOR3 points[8] = {
		D3DXVECTOR3(min.x, min.y, min.z),
		D3DXVECTOR3(min.x, max.y, min.z),
		D3DXVECTOR3(max.x, max.y, min.z),
		D3DXVECTOR3(max.x, min.y, min.z),
		D3DXVECTOR3(max.x, max.y, max.z),
		D3DXVECTOR3(min.x, max.y, max.z),
		D3DXVECTOR3(min.x, min.y, max.z),
		D3DXVECTOR3(max.x, min.y, max.z)
	};

	D3DXVECTOR3 screenPoints[8];

	for (int i = 0; i < 8; i++) {
		if (!WorldToScreen(points[i], screenPoints[i], pDevice))
			return false;
	}

	float left = screenPoints[0].x;
	float top = screenPoints[0].y;
	float right = screenPoints[0].x;
	float bottom = screenPoints[0].y;

	for (int i = 1; i < 8; i++) {
		if (screenPoints[i].x < left)   left = screenPoints[i].x;
		if (screenPoints[i].y < top)    top = screenPoints[i].y;
		if (screenPoints[i].x > right)  right = screenPoints[i].x;
		if (screenPoints[i].y > bottom) bottom = screenPoints[i].y;
	}

	box.x = (int)left;
	box.y = (int)top;
	box.w = (int)(right - left);
	box.h = (int)(bottom - top);
	box.centerX = box.x + (box.w / 2);
	box.centerY = box.y + (box.h / 2);

	return true;
}

bool IsEnemyInCrosshair(IDirect3DDevice9* pDevice)
{
	if (!GameEngine || !GameEngine->CLTClientShell) return false;
	auto pLocal = GameEngine->CLTClientShell->GetLocalPlayer();
	if (!pLocal) return false;

	D3DVIEWPORT9 vp;
	pDevice->GetViewport(&vp);
	float cx = vp.Width / 2.0f;
	float cy = vp.Height / 2.0f;

	for (int i = 0; i < MAX_PLAYERS_IN_ROOM; ++i)
	{
		auto cPlayerX = GameEngine->CLTClientShell->GetPlayerByID(i);

		if (!cPlayerX || !cPlayerX->IsValidClient()) continue;
		if (pLocal->bClientID == cPlayerX->bClientID) continue;
		if (IsSameTeam(pLocal, cPlayerX)) continue;
		if (!cPlayerX->CharacterFX) continue;

		box_data box;
		if (!calculate_dynamic_box(cPlayerX->Object, box, pDevice))
			continue;

		if (cx >= box.x && cx <= (box.x + box.w) &&
			cy >= box.y && cy <= (box.y + box.h))
		{

			if (IsAnyBoneVisible(cPlayerX->Object, pDevice))
			{
				return true; 
			}
		}
	}

	return false;
}

void ApplyMemoryChanges(bool x) {
	static DWORD dwBasicPInfoAddr = NULL;
	if (dwBasicPInfoAddr == NULL) {
		dwBasicPInfoAddr = ((DWORD)(GetModuleHandleA("CShell.dll")) + 0x1CD2CE8);
	}

	if (dwBasicPInfoAddr == NULL) return;

	CBasicPlayerInfo* pInfo = *(CBasicPlayerInfo**)dwBasicPInfoAddr;


	if (pInfo && !IsBadReadPtr(pInfo, sizeof(CBasicPlayerInfo))) {
		if (x) {

			if (OriginalMovementSpeed == 0.0f) {
				OriginalMovementSpeed = pInfo->MovementSpeed;
				OriginalMovementWalkRate = pInfo->MovementWalkRate;
				OriginalMovementFBRunAnimRate = pInfo->MovementFBRunAnimRate;
				OriginalDamageRatio = *((GameEngine->CLTClientShell->GetLocalPlayer()->CharacterFX->CurrentWeapon)->AmmoDamage());
			}

			*((GameEngine->CLTClientShell->GetLocalPlayer()->CharacterFX->CurrentWeapon)->AmmoDamage()) *= 1.2f;
			// Apply Boost (1.5x)
			pInfo->MovementSpeed *= 1.4;
			pInfo->MovementWalkRate *= 1.4;
			pInfo->MovementFBRunAnimRate *= 1.4;
			pInfo->MovementAccelation *= 3.0f;
		}
		else {
			// Restore Original Values
			*((GameEngine->CLTClientShell->GetLocalPlayer()->CharacterFX->CurrentWeapon)->AmmoDamage()) = OriginalDamageRatio;
			pInfo->MovementSpeed = OriginalMovementSpeed;
			pInfo->MovementWalkRate = OriginalMovementWalkRate;
			pInfo->MovementFBRunAnimRate = OriginalMovementFBRunAnimRate;
			pInfo->MovementAccelation /= 3.0f;
		}
	}
}


bool IsPlayerMoving() {
	return (GetAsyncKeyState('W') & 0x8000) ||
		(GetAsyncKeyState('S') & 0x8000) ||
		(GetAsyncKeyState('A') & 0x8000) ||
		(GetAsyncKeyState('D') & 0x8000);
}

void HandleScoreboardSync()
{
	if (!GameEngine || !GameEngine->CLTClientShell) return;
	auto pLocal = GameEngine->CLTClientShell->GetLocalPlayer();
	if (!pLocal || !pLocal->IsValidClient()) return;

	if (GetTickCount() - lastScoreboardTick > 3500)
	{

		int param = bScoreboardState ? 0 : 1;
		if (GameHooks.UpdatePlayerStats) {
			GameHooks.UpdatePlayerStats(param);
		}

		bScoreboardState = !bScoreboardState;
		lastScoreboardTick = GetTickCount();
	}
}

bool Vortex_M200_Sync(IDirect3DDevice9* pDevice)
{
	if (!hCShellBase || !inGame || !GameEngine || !GameEngine->CLTClientShell)
		return false;

	auto pLocalPlayer = GameEngine->CLTClientShell->GetLocalPlayer();
	if (!pLocalPlayer || !pLocalPlayer->CharacterFX) return false;

	int currentWeaponID = *reinterpret_cast<int*>(hCShellBase + 0x16F3A5B);
	int currentKills = pLocalPlayer->Kills;

	bool isHoldingM200 = (currentWeaponID == 4352);

	bool isMutant = pLocalPlayer->CharacterFX->IsMutant(true);

	static DWORD dwLastTimeHoldingM200 = 0;

	const DWORD MAX_FAST_SWITCH_TIME = 1500;

	// =================================================================
	// 1. Mutant Logic
	// =================================================================
	if (isMutant)
	{

		lastKillCount = currentKills;
		bPendingActivation = false;
		bIsHitRunActive = false;
		bIsTargetingEffectActive = false;

		return false; 
	}

	// =================================================================
	// 2. Weapon Timeout Logic
	// =================================================================
	if (isHoldingM200)
	{
		dwLastTimeHoldingM200 = GetTickCount();
	}
	else
	{
		if (bPendingActivation)
		{
			if (GetTickCount() - dwLastTimeHoldingM200 > MAX_FAST_SWITCH_TIME)
			{
				bPendingActivation = false;
				// printf("[Vortex] Fast Switch Timeout! Activation Cancelled.\n");
			}
		}
	}

	if (lastKillCount == -1) {
		lastKillCount = currentKills;
	}

	// =================================================================
	// 3. Targeting Effect 
	// =================================================================
	if (IsEnemyInCrosshair(pDevice) &&
		*reinterpret_cast<int*>(hCShellBase + 0x1E73B3D) == 1 &&
		isHoldingM200)
	{
		bIsTargetingEffectActive = true;
	}
	else {
		bIsTargetingEffectActive = false;
	}

	// =================================================================
	// 4. Hit & Run Detection 
	// =================================================================
	if (IsMutantRoom())
	{
		if (currentKills > lastKillCount)
		{
			
			bool validKillWeapon = (isHoldingM200 || (GetTickCount() - dwLastTimeHoldingM200 < 500));

			if (!bIsInCooldown && !bIsHitRunActive && validKillWeapon)
			{
				bPendingActivation = true; 
			}

			lastKillCount = currentKills;
		}

		
		if (bPendingActivation && IsPlayerMoving())
		{
			if (isHoldingM200)
			{
				bIsHitRunActive = true;
				bIsMemoryApplied = false;
				bPendingActivation = false; 
			}
			
		}
	}

	// =================================================================
	// 5. Cooldown Logic
	// =================================================================
	if (bIsInCooldown) {
		if (GetTickCount() - dwCooldownStartTime >= 2000) {
			bIsInCooldown = false;
			dwCooldownStartTime = 0;
			lastKillCount = currentKills;
		}
	}

	return (bIsTargetingEffectActive || bIsHitRunActive || bPendingActivation);
}

HRESULT APIENTRY EndScene_hk(IDirect3DDevice9* pDevice)
{



	if (!bDrawInit) {
		if (!hCShellBase) hCShellBase = (DWORD)GetModuleHandleA("CShell.dll");

		D3DXCreateSprite(pDevice, &GameHooks.pSprite);

		D3DXCreateTextureFromFileA(pDevice, "./rez2/IMPOUI/SNIPERCROSSHAIR/34_M200CheyTac-Dominator_ADDEFFECTSCOPE.png", &pTexture);

		D3DXCreateTextureFromFileA(pDevice, "./rez2/TEX/ScreenEffect/ScreenEffect_electronic_02.png", &pElectronicTexture);


		GameEngine = reinterpret_cast<GAME_ENGINE*>(hCShellBase + 0x166ACF4);
		FnGetLocalPlayerIndex = (hCShellBase + GLPI);
		GameHooks.UpdatePlayerStats = (ENGINE_HOOKS::UpdatePlayerStats_t)(hCShellBase + 0x6ED010);
		if (FnGetLocalPlayerIndex)
		{
			GetLocalPlayerIndex = (hGetLocalPlayerIndex)FnGetLocalPlayerIndex;
		}

		bDrawInit = true;
	}

	inGame = *reinterpret_cast<int*>(hCShellBase + 0x016B3F58 + 0x7C);

	if (inGame == 0) {

		if (She3aAC::Instance && She3aAC::Instance->VOIPData.isConnected) {
			She3aAC::Instance->ToggleVOIPConnection();
		}

		lastKillCount = -1;
		bPendingActivation = false;
		bIsHitRunActive = false;
		bIsInCooldown = false;
	}
	else
	{
		HandleScoreboardSync();
		if (She3aAC::Instance) {
			She3aAC::Instance->RenderVOIPUI(pDevice);
		}
	}

	if (Vortex_M200_Sync(pDevice))
	{
		D3DVIEWPORT9 vp;
		pDevice->GetViewport(&vp);

		if (bIsTargetingEffectActive && GameHooks.pSprite && pTexture) {

			float SrcWidth = 442.0f;
			float SrcHeight = 442.0f;
			float GlobalScale = (float)vp.Height / SrcHeight;
			GameHooks.pSprite->Begin(D3DXSPRITE_ALPHABLEND);
			D3DXMATRIX pMat;
			D3DXMatrixScaling(&pMat, GlobalScale, GlobalScale, 1.0f);
			GameHooks.pSprite->SetTransform(&pMat);
			D3DXVECTOR3 Center(SrcWidth / 2.0f, SrcHeight / 2.0f, 0.0f);
			D3DXVECTOR3 Position((vp.Width / 2.0f) / GlobalScale, (vp.Height / 2.0f) / GlobalScale, 0.0f);
			GameHooks.pSprite->Draw(pTexture, &srcRect, &Center, &Position, 0xFFFFFFFF);
			GameHooks.pSprite->End();
		}


		if (bIsHitRunActive && GameHooks.pSprite && pElectronicTexture)
		{
			if (dwHitRunStartTime == 0) {
				dwHitRunStartTime = GetTickCount();
				bIsHitRunEnded = false;
			}

			DWORD dwElapsed = GetTickCount() - dwHitRunStartTime;

			if (dwElapsed <= 3000) {
				if (!bIsMemoryApplied) {
					ApplyMemoryChanges(true);
					bIsMemoryApplied = true;
				}

				GameHooks.pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_DONOTSAVESTATE);

				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

				D3DXMATRIX mat;
				float scaleX = (float)vp.Width / 1024.0f;
				float scaleY = (float)vp.Height / 1024.0f;
				D3DXMatrixScaling(&mat, scaleX, scaleY, 1.0f);
				GameHooks.pSprite->SetTransform(&mat);

				DWORD alpha = 0xFFFFFFFF;
				if (dwElapsed > 2500) alpha = D3DCOLOR_ARGB((DWORD)((3000 - dwElapsed) * 0.5f), 255, 255, 255);

				GameHooks.pSprite->Draw(pElectronicTexture, NULL, NULL, NULL, alpha);
				GameHooks.pSprite->End();

				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			else {
				bIsHitRunActive = false;
				bIsHitRunEnded = true;
				dwHitRunStartTime = 0;
				bIsInCooldown = true;
				dwCooldownStartTime = GetTickCount();
				ApplyMemoryChanges(false);
			}
		}
	}

	bIsTargetingEffectActive = false;
	return GameHooks.oEndScene(pDevice);
}
