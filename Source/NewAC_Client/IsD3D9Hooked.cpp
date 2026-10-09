//#include "stdafx.h"
//#include "SAC.h"
//
//
//bool She3aAC::IsD3D9Hooked()
//{
//	if (!D3D9 && !d3dVTable)
//	{
//		if (!D3D9)
//			D3D9 = GetModuleHandleA(d3d9);
//		if (!d3dVTable)
//		{
//			DWORD D3D9_Cur = (DWORD)D3D9;
//			while (D3D9_Cur < (DWORD)D3D9 + 0x127850)
//			{
//				if ((*(WORD*)(D3D9_Cur + 0x00)) == 0x06C7 && (*(WORD*)(D3D9_Cur + 0x06)) == 0x8689 && (*(WORD*)(D3D9_Cur + 0x0C)) == 0x8689)
//				{
//					D3D9_Cur += 2;
//					break;
//				}
//				D3D9_Cur++;
//			}
//			d3dVTable = *(DWORD**)D3D9_Cur;
//		}
//	}
//
//	else
//	{
//		//bool hookedReset = IsTextSectionHookPresent(d3dVTable[16]); // Reset
//		//if (hookedReset)
//		//	return true;
//
//		//bool hookedPresent = IsTextSectionHookPresent(d3dVTable[17]); // Present
//		//if (hookedPresent)
//		//	return true;
//
//		bool hookedBeginScene = IsTextSectionHookPresent(d3dVTable[41]); // BeginScene
//		if (hookedBeginScene)
//			return true;
//
//		bool hookedEndScene = IsTextSectionHookPresent(d3dVTable[42]); // EndScene
//		if (hookedEndScene)
//			return true;
//
//		bool DrawIndexedPrimitivehooked = IsTextSectionHookPresent(d3dVTable[82]); // DrawIndexedPrimitive
//		if (DrawIndexedPrimitivehooked)
//			return true;
//	}
//	return false;
//}
//
//bool She3aAC::IsD3D9Hooked2()
//{
//	if (!D3D9 && !d3dVTable)
//	{
//		if (!D3D9)
//			D3D9 = GetModuleHandleA(d3d9);
//		if (!d3dVTable)
//		{
//			DWORD D3D9_Cur = (DWORD)D3D9;
//			while (D3D9_Cur < (DWORD)D3D9 + 0x127850)
//			{
//				if ((*(WORD*)(D3D9_Cur + 0x00)) == 0x06C7 && (*(WORD*)(D3D9_Cur + 0x06)) == 0x8689 && (*(WORD*)(D3D9_Cur + 0x0C)) == 0x8689)
//				{
//					D3D9_Cur += 2;
//					break;
//				}
//				D3D9_Cur++;
//			}
//			d3dVTable = *(DWORD**)D3D9_Cur;
//		}
//	}
//
//	else
//	{
//		bool hookedReset = IsTextSectionHookPresent(d3dVTable[16]); // Reset
//		if (hookedReset)
//			return true;
//
//		bool hookedPresent = IsTextSectionHookPresent(d3dVTable[17]); // Present
//		if (hookedPresent)
//			return true;
//
//		bool Clear = IsTextSectionHookPresent(d3dVTable[43]); // Clear 
//		if (Clear)
//			return true;
//
//		bool GetTransform = IsTextSectionHookPresent(d3dVTable[45]); // GetTransform 
//		if (GetTransform)
//			return true;
//
//		bool GetViewport = IsTextSectionHookPresent(d3dVTable[48]); // GetViewport 
//		if (GetViewport)
//			return true;
//
//		bool SetRenderState = IsTextSectionHookPresent(d3dVTable[57]); // SetRenderState 
//		if (SetRenderState)
//			return true;
//
//		bool SetTexture = IsTextSectionHookPresent(d3dVTable[65]); // SetTexture 
//		if (SetTexture)
//			return true;
//
//		bool DrawPrimitiveUP = IsTextSectionHookPresent(d3dVTable[83]); // DrawPrimitiveUP 
//		if (DrawPrimitiveUP)
//			return true;
//
//		bool SetFVF = IsTextSectionHookPresent(d3dVTable[89]); // SetFVF  
//		if (SetFVF)
//			return true;
//
//		bool hookedSetStreamSource = IsTextSectionHookPresent(d3dVTable[100]); // StreamSourcehooked
//		if (hookedSetStreamSource)
//			return true;
//
//		bool SetPixelShader = IsTextSectionHookPresent(d3dVTable[107]); // SetPixelShader
//		if (SetPixelShader)
//			return true;
//	}
//	return false;
//}