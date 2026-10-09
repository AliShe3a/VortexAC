#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsD3D9Hooked()
{
	if (!D3D9 && !d3dVTable)
	{
		if (!D3D9)
			D3D9 = GetModuleHandleA(d3d9);
		if (!d3dVTable)
		{
			DWORD D3D9_Cur = (DWORD)D3D9;
			while (D3D9_Cur < (DWORD)D3D9 + 0x127850)
			{
				if ((*(WORD*)(D3D9_Cur + 0x00)) == 0x06C7 && (*(WORD*)(D3D9_Cur + 0x06)) == 0x8689 && (*(WORD*)(D3D9_Cur + 0x0C)) == 0x8689)
				{
					D3D9_Cur += 2;
					break;
				}
				D3D9_Cur++;
			}
			d3dVTable = *(DWORD**)D3D9_Cur;
		}
	}

	else
	{
		bool hookedReset = ValidateAddress(d3dVTable[16], d3d9); // Reset
		if (hookedReset)
			return true;

		bool hookedPresent = ValidateAddress(d3dVTable[17], d3d9); // Present
		if (hookedPresent)
			return true;

		bool hookedBeginScene = ValidateAddress(d3dVTable[41], d3d9); // BeginScene
		if (hookedBeginScene)
			return true;

		bool hookedEndScene = ValidateAddress(d3dVTable[42], d3d9); // EndScene
		if (hookedEndScene)
			return true;

		bool DrawIndexedPrimitivehooked = ValidateAddress(d3dVTable[82], d3d9); // DrawIndexedPrimitive
		if (DrawIndexedPrimitivehooked)
			return true;

		bool SetTexture = ValidateAddress(d3dVTable[65], d3d9); // SetTexture 
		if (SetTexture)
			return true;

		bool SetRenderState = ValidateAddress(d3dVTable[57], d3d9); // SetRenderState 
		if (SetRenderState)
			return true;
	}
	return false;
}

bool She3aAC::IsD3D9Hooked2()
{
	if (!D3D9 && !d3dVTable)
	{
		if (!D3D9)
			D3D9 = GetModuleHandleA(d3d9);
		if (!d3dVTable)
		{
			DWORD D3D9_Cur = (DWORD)D3D9;
			while (D3D9_Cur < (DWORD)D3D9 + 0x127850)
			{
				if ((*(WORD*)(D3D9_Cur + 0x00)) == 0x06C7 && (*(WORD*)(D3D9_Cur + 0x06)) == 0x8689 && (*(WORD*)(D3D9_Cur + 0x0C)) == 0x8689)
				{
					D3D9_Cur += 2;
					break;
				}
				D3D9_Cur++;
			}
			d3dVTable = *(DWORD**)D3D9_Cur;
		}
	}

	else
	{
		bool hookedReset = ValidateAddress(d3dVTable[16], d3d9); // Reset
		if (hookedReset)
			return true;

		bool hookedPresent = ValidateAddress(d3dVTable[17], d3d9); // Present
		if (hookedPresent)
			return true;

		bool Clear = ValidateAddress(d3dVTable[43], d3d9); // Clear
		if (Clear)
			return true;

		bool GetTransform = ValidateAddress(d3dVTable[45], d3d9); // GetTransform
		if (GetTransform)
			return true;

		bool GetViewport = ValidateAddress(d3dVTable[48], d3d9); // GetViewport
		if (GetViewport)
			return true;

		bool SetRenderState = ValidateAddress(d3dVTable[57], d3d9); // SetRenderState
		if (SetRenderState)
			return true;

		bool SetTexture = ValidateAddress(d3dVTable[65], d3d9); // SetTexture
		if (SetTexture)
			return true;

		bool DrawPrimitiveUP = ValidateAddress(d3dVTable[83], d3d9); // DrawPrimitiveUP
		if (DrawPrimitiveUP)
			return true;

		bool SetFVF = ValidateAddress(d3dVTable[89], d3d9); // SetFVF
		if (SetFVF)
			return true;

		bool hookedSetStreamSource = ValidateAddress(d3dVTable[100], d3d9); // StreamSourcehooked
		if (hookedSetStreamSource)
			return true;

		bool SetPixelShader = ValidateAddress(d3dVTable[107], d3d9); // SetPixelShader
		if (SetPixelShader)
			return true;
	}
	return false;
}