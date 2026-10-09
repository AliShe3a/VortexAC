#include "SAC.h"
#include <windows.h>

// --- ITEM.CFT --- //

#define ITEMS_COUNT 5120 * 2
#define ITEMS_SIZE 0xEA8

// --- BF005.LTC --- //

#define WEAPONS_COUNT 0x1000 * 2
#define WEAPONS_SIZE 0x479C
#define WEAPONS_SIZE_OBJ 0x1634

void She3aAC::ItemLimitPatch()
{

	DWORD hCshell = cPlayer->cEngine->GetCShellDLL();
	DWORD CROSSFIRE2 = cPlayer->cEngine->CrossFire;

	if (!hCshell)
		return;

	// --- ITEM LIMIT ARRAY --- //

	DWORD ITEM_LIMIT_ARRAY[] = {

		hCshell + 0x7B801B + 0x2,
		hCshell + 0x7B7FCA + 0x2,
		hCshell + 0x7B7F39 + 0x2,
		hCshell + 0x7B7EB1 + 0x2,
		hCshell + 0x7B7DF9 + 0x2,
		hCshell + 0x7B7346 + 0x2,
		hCshell + 0x554F8C + 0x1,
		hCshell + 0x5489C2 + 0x2,
		hCshell + 0x47D96A + 0x1,
		hCshell + 0x461E53 + 0x2,
		hCshell + 0x39FD1F + 0x2,
		hCshell + 0x398287 + 0x2,
		hCshell + 0x2FAEC3 + 0x2,
		hCshell + 0x2F8CCB + 0x2,
		hCshell + 0x27B716 + 0x2,
		hCshell + 0x2622EA + 0x2,
		hCshell + 0xFA788 + 0x2,

	};

	// --- ITEM LIMIT INDEX --- //

	DWORD ITEM_LIMIT[] = {
		hCshell + 0x106C7F + 0x1,
		hCshell + 0x1A0C4C + 0x1,
		hCshell + 0x1A0F1F + 0x2,
		hCshell + 0x1B0636 + 0x1,
		hCshell + 0x1B082A + 0x1,
		hCshell + 0x1B49BD + 0x1,
		hCshell + 0x214D8F + 0x1,
		hCshell + 0x214E17 + 0x1,
		hCshell + 0x4F0AB4 + 0x1,
		hCshell + 0x5549C9 + 0x2,
		hCshell + 0x7B73AB + 0x1,
		hCshell + 0x7B7513 + 0x1,
		hCshell + 0x7B75F3 + 0x1,
		hCshell + 0x7B7980 + 0x1,
		hCshell + 0x7B7A72 + 0x1,
		hCshell + 0x7B7B02 + 0x2,
		hCshell + 0x106BCB + 0x1,
	};

	// --- ITEM LIMIT INDEX MINUS ONE --- //

	DWORD ITEM_LIMIT_MINUS_ONE[] = {
		hCshell + 0x7B72D6,
		hCshell + 0x122665,
		hCshell + 0x13F904 + 0x1,
		hCshell + 0x24A5C6 + 0x2,
		hCshell + 0x264A66 + 0x2,
		hCshell + 0x55518F + 0x1,
		hCshell + 0x7B4D3E + 0x1,
		hCshell + 0x7B5A86 + 0x1,
		hCshell + 0x106BB8
	};


	// --- WEAPON LIMIT INDEX --- //

	DWORD WEAPON_LIMIT[] = {
		hCshell + 0x5756c4,
		hCshell + 0x5636B1,
		hCshell + 0x56c849,
		hCshell + 0x56cb9B,
		hCshell + 0x56d0A2,
		hCshell + 0x575b15,
		hCshell + 0x1a7e0d,

	};


	// --- WEAPON LIMIT INDEX MINUS ONE --- //

	DWORD WEAPON_LIMIT_MINUS_ONE[] = {
		hCshell + 0x56379D,
		hCshell + 0x56372B,
		hCshell + 0x56d361,
		hCshell + 0x5636ee,
		hCshell + 0x56c603,

	};

	// --- PATCHING ITEM.CFT --- //

	for (int i = 0; i < sizeof(ITEM_LIMIT_ARRAY) / sizeof(ITEM_LIMIT_ARRAY[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(ITEM_LIMIT_ARRAY[i], (ITEMS_COUNT * ITEMS_SIZE));
	}


	for (int i = 0; i < sizeof(ITEM_LIMIT) / sizeof(ITEM_LIMIT[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(ITEM_LIMIT[i], ITEMS_COUNT);
	}


	for (int i = 0; i < sizeof(ITEM_LIMIT_MINUS_ONE) / sizeof(ITEM_LIMIT_MINUS_ONE[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(ITEM_LIMIT_MINUS_ONE[i], (ITEMS_COUNT - 1));
	}

	cPlayer->cEngine->vWPM<short>((hCshell + 0x5535FA), (ITEMS_COUNT - 1));
	cPlayer->cEngine->vWPM<short>((hCshell + (0x7B3349 + 0x2)), ITEMS_COUNT);
	cPlayer->cEngine->vWPM<short>((hCshell + (0x7B3389 + 0x2)), ITEMS_COUNT);

	// --- PATCHING BF005.LTC --- //

	cPlayer->cEngine->vWPM<int>((hCshell + 0x563642), (WEAPONS_COUNT * WEAPONS_SIZE));

	for (int i = 0; i < sizeof(WEAPON_LIMIT) / sizeof(WEAPON_LIMIT[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(WEAPON_LIMIT[i], WEAPONS_COUNT);
	}

	for (int i = 0; i < sizeof(WEAPON_LIMIT_MINUS_ONE) / sizeof(WEAPON_LIMIT_MINUS_ONE[0]); ++i) {
		cPlayer->cEngine->vWPM<short>(WEAPON_LIMIT_MINUS_ONE[i], (WEAPONS_COUNT - 1));
	}

}

bool She3aAC::ItemLimitPatch_inGame() {
	// --- ITEM LIMIT INDEX BOT MATCH --- //
	DWORD hObject = cPlayer->cEngine->GetObjectDLL();

	if (!hObject)
		return false;

	// --- ITEM LIMIT ARRAY --- //

	DWORD ITEM_LIMIT_BOT_MATCH_ARRAY[] = {
	hObject + 0x6D0C5 + 0x1,
	hObject + 0x6B1D3 + 0x1,
	hObject + 0x6B1E6 + 0x1,
	hObject + 0x6CF2D + 0x4,
	hObject + 0x6D8C0 + 0x4,
	hObject + 0xC52EF + 0x2,
	};

	// --- ITEM LIMIT INDEX --- //

	DWORD ITEM_LIMIT_BOT_MATCH[] = {
	hObject + 0x6D0E8 + 0x1,
	hObject + 0xC5372 + 0x2,
	hObject + 0xF4F2D + 0x1,
	hObject + 0xF51BA + 0x1,
	hObject + 0x571EDB + 0x1,
	hObject + 0x571FB3 + 0x1,

	hObject + 0x56838B + 0x2,
	hObject + 0x568561 + 0x3,
	hObject + 0x568579 + 0x3,
	hObject + 0x568586 + 0x3,
	hObject + 0x5685E7 + 0x3,
	hObject + 0x568657 + 0x3,
	};

	// --- ITEM LIMIT INDEX MINUS ONE --- //

	DWORD ITEM_LIMIT_BOT_MATCH_MINUS_ONE[] = {
	hObject + 0x6D2D8 + 0x1,
	hObject + 0x571E74 + 0x2,

	};

	// --- WEAPON LIMIT INDEX --- //

	DWORD WEAPON_BOT_MATCH_LIMIT[] = {
		hObject + 0x1d6c7e,
		hObject + 0x1d61fd,
		hObject + 0x1d4bb9,
		hObject + 0x1d24c3,
		hObject + 0x1c34b2,
		hObject + 0x8c756,
		hObject + 0x7a421,
	};


	// --- WEAPON LIMIT INDEX MINUS ONE --- //

	DWORD WEAPON_LIMIT_BOT_MATCH_MINUS_ONE[] = {
		hObject + 0x7a45e,
		hObject + 0x7a49b,
		hObject + 0x7a52d,
		hObject + 0x832a3,
		hObject + 0x84001,
		hObject + 0x1c1d2d,
		hObject + 0x1c5307,
	};

	// --- PATCHING ITEM.CFT --- //

	for (int i = 0; i < sizeof(ITEM_LIMIT_BOT_MATCH_ARRAY) / sizeof(ITEM_LIMIT_BOT_MATCH_ARRAY[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(ITEM_LIMIT_BOT_MATCH_ARRAY[i], (ITEMS_COUNT * ITEMS_SIZE));
	}

	for (int i = 0; i < sizeof(ITEM_LIMIT_BOT_MATCH) / sizeof(ITEM_LIMIT_BOT_MATCH[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(ITEM_LIMIT_BOT_MATCH[i], (ITEMS_COUNT));
	}

	for (int i = 0; i < sizeof(ITEM_LIMIT_BOT_MATCH_MINUS_ONE) / sizeof(ITEM_LIMIT_BOT_MATCH_MINUS_ONE[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(ITEM_LIMIT_BOT_MATCH_MINUS_ONE[i], (ITEMS_COUNT - 1));
	}

	cPlayer->cEngine->vWPM<short>((hObject + (0x6B137 + 0x3)), (ITEMS_COUNT - 1), 2);


	// --- PATCHING BF005.LTC --- //

	cPlayer->cEngine->vWPM<int>((hObject + 0x7a3b2), (WEAPONS_COUNT * WEAPONS_SIZE));



	cPlayer->cEngine->vWPM<short>((hObject + 0x1f1397), (WEAPONS_COUNT));

	for (int i = 0; i < sizeof(WEAPON_BOT_MATCH_LIMIT) / sizeof(WEAPON_BOT_MATCH_LIMIT[0]); ++i) {
		cPlayer->cEngine->vWPM<int>(WEAPON_BOT_MATCH_LIMIT[i], WEAPONS_COUNT);
	}


	for (int i = 0; i < sizeof(WEAPON_LIMIT_BOT_MATCH_MINUS_ONE) / sizeof(WEAPON_LIMIT_BOT_MATCH_MINUS_ONE[0]); ++i) {
		cPlayer->cEngine->vWPM<short>(WEAPON_LIMIT_BOT_MATCH_MINUS_ONE[i], (WEAPONS_COUNT - 1));
	}

	return true;
}
