#define NOMINMAX

#include "thprac_init.h"
#include "thprac_launcher_main.h"
#include "thprac_launcher_cfg.h"
#include "thprac_log.h"
#include "thprac_main.h"
#include "thprac_gui_locale.h"
#include "thprac_hook.h"
#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#define MB_INFO(str) MessageBoxA(NULL, str, str, MB_OK);

using namespace THPrac;

bool PrivilegeCheck()
{
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    HookCtx::VEHInit();

    auto thpracMutex = OpenMutexW(SYNCHRONIZE, FALSE, L"thprac - Touhou Practice Tool##mutex");
    RemoteInit();
    //SetCurrentDirectoryA("D:\\Touhou\\Games\\th18");
    //SetCurrentDirectoryA("D:\\Projects\\thprac\\Debug");

    bool adminRights = false;
    if (LauncherCfgInit(true)) {
        if (!Gui::LocaleInitFromCfg()) {
            Gui::LocaleAutoSet();
        }
        LauncherSettingGet("thprac_admin_rights", adminRights);
        LauncherCfgClose();
    }

    if (adminRights && !PrivilegeCheck()) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        CloseHandle(thpracMutex);
        ShellExecuteA(NULL, "runas", exePath, NULL, NULL, SW_SHOW);
        return 0;
    }

    return GuiLauncherMain();
}