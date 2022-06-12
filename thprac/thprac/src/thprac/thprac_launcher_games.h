#pragma once
#include "utils/utils.h"
#include "thprac_gui_locale.h"
#include "thprac_launcher_games_def.h"
#include <cstdint>
#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace THPrac {

bool GetExeInfo(void* exeBuffer, size_t exeSize, ExeSig& exeSigOut);
bool GetExeInfoEx(size_t process, ExeSig& exeSigOut);

bool LauncherGamesGuiUpd();
void LauncherGamesGuiSwitch(const char* idStr);
void LauncherGamesForceReload();
void* LauncherGamesExternalLaunch(const char* game, std::string ctx, int type = TYPE_ORIGINAL);
}