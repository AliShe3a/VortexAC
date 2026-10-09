#pragma once
#include <winnt.h>
#include <minwindef.h>
#include <winnt.h>
#include <intrin.h>
typedef struct _LDR_MODULE {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID BaseAddress;
	PVOID EntryPoint;
	ULONG SizeOfImage;


}LDR_MODULE, * PLDR_MODULE;

typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;

}PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;

}PEB, * PPEB;

void HideModule(HMODULE module)
{
	PEB* peb;
	LDR_MODULE* ldr;

	peb = (PEB*)__readfsdword(0x30);

	ldr = (LDR_MODULE*)peb->Ldr->InLoadOrderModuleList.Flink;

	while (ldr->BaseAddress != 0)
	{
		if (ldr->BaseAddress == module)
		{
			if (ldr->InLoadOrderModuleList.Blink != 0)
				(ldr->InLoadOrderModuleList.Blink)->Flink = ldr->InLoadOrderModuleList.Flink;
			if (ldr->InLoadOrderModuleList.Blink != 0)
				(ldr->InLoadOrderModuleList.Flink)->Blink = ldr->InLoadOrderModuleList.Blink;

			if (ldr->InInitializationOrderModuleList.Blink != 0)
				(ldr->InInitializationOrderModuleList.Blink)->Flink = ldr->InInitializationOrderModuleList.Flink;
			if (ldr->InInitializationOrderModuleList.Flink != 0)
				(ldr->InInitializationOrderModuleList.Flink)->Blink = ldr->InInitializationOrderModuleList.Blink;

			if (ldr->InMemoryOrderModuleList.Flink != 0)
				(ldr->InMemoryOrderModuleList.Blink)->Flink = ldr->InMemoryOrderModuleList.Flink;
			if (ldr->InMemoryOrderModuleList.Flink != 0)
				(ldr->InMemoryOrderModuleList.Flink)->Blink = ldr->InMemoryOrderModuleList.Blink;
		}
		ldr = (LDR_MODULE*)ldr->InLoadOrderModuleList.Flink;
	}

}