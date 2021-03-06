#if 0

#include <windows.h>
#include <SDKDDKVer.h>
#include "thprac_init.h"
#include <string>


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	std::string s;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		THPrac::RemoteInit();
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif


#if 0
#include "dll_hack.h"
#pragma comment(linker, "/EXPORT:DirectInput8Create=_DirectInput8Create@20")
extern "C" HRESULT WINAPI DirectInput8Create(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID *ppvOut,
	void* punkOuter)
{
	DLL_REPLACE_FUNC(DirectInput8Create, L"dinput8.dll", "DirectInput8Create");
	return DLL_CALL_ORIG(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
#endif

