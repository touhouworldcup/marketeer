#include "thprac_launcher_cfg.h"
#include "thprac_gui_locale.h"
#include "thprac_launcher_main.h"
#include "thprac_launcher_games.h"
#include "thprac_launcher_utils.h"
#include "thprac_licence.h"
#include "thprac_log.h"
#include "thprac_version.h"
#include "utils/utils.h"
#include "..\..\resource.h"
#include <Windows.h>
#include <functional>
#include <string>
#include <vector>
#include <winhttp.h>
#pragma warning(disable : 4091)
#include <ShlObj.h>
#pragma warning(default : 4091)
#include <rapidjson/prettywriter.h>
#pragma comment(lib, "winhttp.lib")
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace THPrac {
std::wstring* gCfgPath = nullptr;
int gCfgIsLocalDir = 0;
bool gCfgLocalBackupAvaliable = false;
bool gCfgReadOnly = false;
HANDLE gCfgHnd = INVALID_HANDLE_VALUE;
rapidjson::Document gCfgJson;

void LauncherAquireDataDirVar()
{
    if (gCfgPath) {
        return;
    }
    gCfgPath = new std::wstring();

    std::wstring path;
    wchar_t pathBuffer[MAX_PATH];
    GetModuleFileNameW((HMODULE)&__ImageBase, pathBuffer, MAX_PATH);
    path = GetDirFromFullPath(std::wstring(pathBuffer));

    auto backupPath = path + L".marketeer_data_backup";
    auto backupPathAttr = GetFileAttributesW(backupPath.c_str());
    if (backupPathAttr != INVALID_FILE_ATTRIBUTES && backupPathAttr & FILE_ATTRIBUTE_DIRECTORY) {
        gCfgLocalBackupAvaliable = true;
    }

    path += L".marketeer_data";
    auto fileAttr = GetFileAttributesW(path.c_str());
    if (fileAttr != INVALID_FILE_ATTRIBUTES && fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
        gCfgIsLocalDir = 1;
        path += L"\\";
    } else {
        gCfgIsLocalDir = 0;
        wchar_t appDataPath[MAX_PATH];
        if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath) == S_OK) {
            path = appDataPath;
            path += L"\\marketeer\\";
        } else {
            path = L"";
        }
    }

    *gCfgPath = path;
}
std::wstring LauncherGetDataDir()
{
    LauncherAquireDataDirVar();

    return *gCfgPath;
}

bool LauncherCfgWrite()
{
    if (gCfgHnd == INVALID_HANDLE_VALUE) {
        return false;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    gCfgJson.Accept(writer);

    auto jsonStr = sb.GetString();
    auto jsonStrLen = strlen(jsonStr);
    DWORD bytesProcessed;
    SetFilePointer(gCfgHnd, 0, NULL, FILE_BEGIN);
    SetEndOfFile(gCfgHnd);
    return WriteFile(gCfgHnd, jsonStr, jsonStrLen, &bytesProcessed, NULL);
}
rapidjson::Document& LauncherCfgGet()
{
    return gCfgJson;
}
bool LauncherCfgInit(bool noCreate)
{
    if (gCfgHnd != INVALID_HANDLE_VALUE) {
        return true;
    }

    std::wstring jsonPath = LauncherGetDataDir();
    CreateDirectoryW(jsonPath.c_str(), NULL);
    jsonPath += L"marketeer.json";

    DWORD openFlag = noCreate ? OPEN_EXISTING : OPEN_ALWAYS;
    DWORD openAccess = noCreate ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE;
    gCfgHnd = CreateFileW(jsonPath.c_str(), openAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, openFlag, FILE_ATTRIBUTE_NORMAL, NULL);
    if (gCfgHnd == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesProcessed;
    auto fileSize = GetFileSize(gCfgHnd, NULL);
    auto fileBuffer = malloc(fileSize + 1);
    memset(fileBuffer, 0, fileSize + 1);
    if (!ReadFile(gCfgHnd, fileBuffer, fileSize, &bytesProcessed, NULL)) {
        return false;
    }

    if (gCfgJson.Parse((const char*)fileBuffer, fileSize + 1).HasParseError()) {
        gCfgJson.SetObject();
        return LauncherCfgWrite();
    }

    gCfgReadOnly = noCreate;
    return true;
}
void LauncherCfgClose()
{
    if (gCfgHnd != INVALID_HANDLE_VALUE) {
        if (!gCfgReadOnly) {
            LauncherCfgWrite();
        }
        CloseHandle(gCfgHnd);
        auto err = GetLastError();
        gCfgReadOnly = false;
        gCfgHnd = INVALID_HANDLE_VALUE;
        delete gCfgPath;
        gCfgPath = nullptr;
    }
}

rapidjson::Value& GetCfgSettingsJson()
{
    if (gCfgJson.HasMember("settings")) {
        if (!gCfgJson["settings"].IsObject()) {
            gCfgJson["settings"].SetObject();
        }
    } else {
        rapidjson::Document settingsJson;
        settingsJson.SetObject();
        JsonAddMemberA(gCfgJson, "settings", settingsJson, gCfgJson.GetAllocator());
    }
    return gCfgJson["settings"];
}
bool LauncherSettingGet(const char* name, int& valueOut)
{
    if (gCfgJson.HasMember("settings") && gCfgJson["settings"].IsObject()) {
        auto& settingsJson = gCfgJson["settings"];
        if (settingsJson.HasMember(name) && settingsJson[name].IsInt()) {
            valueOut = settingsJson[name].GetInt();
            return true;
        }
    }
    return false;
}
bool LauncherSettingGet(const char* name, unsigned int& valueOut)
{
    if (gCfgJson.HasMember("settings") && gCfgJson["settings"].IsObject()) {
        auto& settingsJson = gCfgJson["settings"];
        if (settingsJson.HasMember(name) && settingsJson[name].IsUint()) {
            valueOut = settingsJson[name].GetUint();
            return true;
        }
    }
    return false;
}
bool LauncherSettingGet(const char* name, bool& valueOut)
{
    if (gCfgJson.HasMember("settings") && gCfgJson["settings"].IsObject()) {
        auto& settingsJson = gCfgJson["settings"];
        if (settingsJson.HasMember(name) && settingsJson[name].IsBool()) {
            valueOut = settingsJson[name].GetBool();
            return true;
        }
    }
    return false;
}
bool LauncherSettingGet(const char* name, float& valueOut)
{
    if (gCfgJson.HasMember("settings") && gCfgJson["settings"].IsObject()) {
        auto& settingsJson = gCfgJson["settings"];
        if (settingsJson.HasMember(name) && settingsJson[name].IsFloat()) {
            valueOut = settingsJson[name].GetFloat();
            return true;
        }
    }
    return false;
}
bool LauncherSettingGet(const char* name, std::string& valueOut)
{
    if (gCfgJson.HasMember("settings") && gCfgJson["settings"].IsObject()) {
        auto& settingsJson = gCfgJson["settings"];
        if (settingsJson.HasMember(name) && settingsJson[name].IsString()) {
            valueOut = settingsJson[name].GetString();
            return true;
        }
    }
    return false;
}
void LauncherSettingSet(const char* name, int& valueIn)
{
    auto& settingsJson = GetCfgSettingsJson();
    if (settingsJson.HasMember(name)) {
        settingsJson.RemoveMember(name);
    }
    JsonAddMember(settingsJson, name, valueIn, gCfgJson.GetAllocator());
    LauncherCfgWrite();
}
void LauncherSettingSet(const char* name, unsigned int& valueIn)
{
    auto& settingsJson = GetCfgSettingsJson();
    if (settingsJson.HasMember(name)) {
        settingsJson.RemoveMember(name);
    }
    JsonAddMember(settingsJson, name, valueIn, gCfgJson.GetAllocator());
    LauncherCfgWrite();
}
void LauncherSettingSet(const char* name, bool& valueIn)
{
    auto& settingsJson = GetCfgSettingsJson();
    if (settingsJson.HasMember(name)) {
        settingsJson.RemoveMember(name);
    }
    JsonAddMember(settingsJson, name, valueIn, gCfgJson.GetAllocator());
    LauncherCfgWrite();
}
void LauncherSettingSet(const char* name, float& valueIn)
{
    auto& settingsJson = GetCfgSettingsJson();
    if (settingsJson.HasMember(name)) {
        settingsJson.RemoveMember(name);
    }
    JsonAddMember(settingsJson, name, valueIn, gCfgJson.GetAllocator());
    LauncherCfgWrite();
}
void LauncherSettingSet(const char* name, std::string& valueIn)
{
    auto& settingsJson = GetCfgSettingsJson();
    if (settingsJson.HasMember(name)) {
        settingsJson.RemoveMember(name);
    }
    JsonAddMemberA(settingsJson, name, valueIn.c_str(), gCfgJson.GetAllocator());
    LauncherCfgWrite();
}

template <typename T>
class THSetting {
public:
    THSetting() = delete;
    THSetting(const char* _name, T _value)
    {
        name = _name;
        value = _value;
        Init();
    }
    virtual void Init() final
    {
        if (!LauncherSettingGet(name.c_str(), value)) {
            LauncherSettingSet(name.c_str(), value);
        }
    }
    virtual T& Get()
    {
        return value;
    }
    virtual void Set(T& _value)
    {
        value = _value;
        LauncherSettingSet(name.c_str(), value);
    }
    virtual void Set()
    {
        LauncherSettingSet(name.c_str(), value);
    }

protected:
    std::string name;
    T value;
};

template <>
class THSetting<std::string> {
public:
    THSetting() = delete;
    THSetting(const char* _name, const char* _value)
    {
        name = _name;
        value = _value;
        Init();
    }
    THSetting(const char* _name, std::string _value)
    {
        name = _name;
        value = _value;
        Init();
    }
    virtual void Init() final
    {
        if (!LauncherSettingGet(name.c_str(), value)) {
            LauncherSettingSet(name.c_str(), value);
        }
    }
    virtual std::string& Get()
    {
        return value;
    }
    virtual void Set(std::string& _value)
    {
        value = _value;
        LauncherSettingSet(name.c_str(), value);
    }
    virtual void Set()
    {
        LauncherSettingSet(name.c_str(), value);
    }

protected:
    std::string name;
    std::string value;
};

class THCfgCheckbox : public THSetting<bool> {
public:
    THCfgCheckbox(const char* _name, bool _value)
        : THSetting(_name, _value)
    {
    }
    void Gui(const char* guiTxt, const char* helpTxt = nullptr)
    {
        if (ImGui::Checkbox(guiTxt, &value)) {
            LauncherSettingSet(name.c_str(), value);
        }
        if (helpTxt) {
            ImGui::SameLine();
            GuiHelpMarker(helpTxt);
        }
    }
};

class THCfgCombo : public THSetting<int> {
public:
    THCfgCombo(const char* _name, int _value, int _counts)
        : THSetting(_name, _value)
    {
        counts = _counts;
        Check(_value);
    }
    void Check(int def = 0)
    {
        if (value < 0 || value >= counts) {
            value = def;
        }
    }
    virtual void Set(int& _value) override
    {
        value = _value;
        LauncherSettingSet(name.c_str(), value);
        Check();
    }
    bool Gui(const char* guiTxt, const char* guiCombo, const char* helpTxt = nullptr)
    {
        ImGui::Text(guiTxt);
        ImGui::SameLine();
        std::string txtTmp = "##";
        txtTmp += guiTxt;
        ImGui::PushItemWidth(620.0f - ImGui::GetWindowSize().x);
        auto result = ImGui::Combo(txtTmp.c_str(), &value, guiCombo);
        ImGui::PopItemWidth();
        if (result) {
            LauncherSettingSet(name.c_str(), value);
        }
        if (helpTxt) {
            ImGui::SameLine();
            GuiHelpMarker(helpTxt);
        }
        return result;
    }

private:
    int counts;
};

class THCfgGui {
private:
    THCfgGui()
    {
        mGuiUpdFunc = [&]() { GuiMain(); };
        CfgInit();
    }
    SINGLETON(THCfgGui);

public:
    void GuiUpdate()
    {
        mGuiUpdFunc();
    }

    void FileOperateWrapper(FILEOP_FLAGS op, const wchar_t* from, const wchar_t* to)
    {
        SHFILEOPSTRUCTW fileStruct;
        fileStruct.hwnd = 0;
        fileStruct.wFunc = op;
        fileStruct.pFrom = from;
        fileStruct.pTo = to;
        fileStruct.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMMKDIR;
        SHFileOperationW(&fileStruct);
    }
    void LauncherCfgReset()
    {
        LauncherCfgClose();
        LogClose();
        if (mCfgResetFlag == 1) {
            wchar_t appDataPath[MAX_PATH];
            if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath) == S_OK) {
                std::wstring jsonPath = appDataPath;
                jsonPath += L"\\marketeer";
                jsonPath += L"\\marketeer.json";
                DeleteFileW(jsonPath.c_str());
            }
        } else if (mCfgResetFlag == 2) {
            if (mCfgUseLocalDir != gCfgIsLocalDir) {
                wchar_t pathBuffer[MAX_PATH];
                wchar_t appDataPath[MAX_PATH];
                SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath);
                GetModuleFileNameW((HMODULE)&__ImageBase, pathBuffer, MAX_PATH);
                std::wstring globalPath = appDataPath;
                globalPath += L"\\marketeer";
                globalPath.push_back('\0');
                globalPath.push_back('\0');
                std::wstring localPath = GetDirFromFullPath(std::wstring(pathBuffer));

                if (!gCfgIsLocalDir) {
                    auto dest = localPath + L".marketeer_data";
                    dest.push_back('\0');
                    dest.push_back('\0');
                    if (mCfgDirChgGlobalParam == 1 || mCfgDirChgGlobalParam == 2) {
                        FileOperateWrapper(FO_DELETE, dest.c_str(), nullptr);
                        FileOperateWrapper(mCfgDirChgGlobalParam == 1 ? FO_MOVE : FO_COPY, globalPath.c_str(), dest.c_str());
                    } else if (mCfgDirChgLocalParam == 1) {
                        auto src = localPath + L".marketeer_data_backup";
                        MoveFileW(src.c_str(), dest.c_str());
                    } else if (mCfgDirChgLocalParam == 0) {
                        CreateDirectoryW(dest.c_str(), NULL);
                    }
                } else {
                    auto src = localPath + L".marketeer_data";
                    src.push_back('\0');
                    src.push_back('\0');
                    if (mCfgDirChgLocalParam == 0) {
                        auto dest = localPath + L".marketeer_data_backup";
                        MoveFileW(src.c_str(), dest.c_str());
                    } else if (mCfgDirChgLocalParam == 1 || mCfgDirChgLocalParam == 2) {
                        if (mCfgDirChgLocalParam == 1) {
                            FileOperateWrapper(FO_DELETE, globalPath.c_str(), nullptr);
                        }
                        FileOperateWrapper(mCfgDirChgLocalParam == 1 ? FO_MOVE : FO_DELETE, src.c_str(), mCfgDirChgLocalParam == 1 ? globalPath.c_str() : nullptr);
                    }
                }
            }

            if (LauncherCfgInit()) {
                bool useRelPath = mCfgRelativePath && gCfgIsLocalDir;
                LauncherSettingSet("use_relative_path", useRelPath);
                LauncherGamesForceReload();
                LauncherCfgClose();
            }
        }
        mCfgResetFlag = 0;
    }

private:
    void CfgInit()
    {
        int language = Gui::LocaleGet();
        mCfgLanguage.Set(language);
        mOriginalLanguage = mCfgLanguage.Get();
    }

    void PathAndDirSettings()
    {
        bool dirSettingModalFlag = false;
        bool confirmModalFlag = false;

        if (ImGui::Button(XSTR(THPRAC_DIRECTORY_SETTING))) {
            ImGui::OpenPopup(XSTR(THPRAC_DIRECTORY_SETTING_MODAL));
            mCfgUseLocalDir = gCfgIsLocalDir;
            mCfgDirChgGlobalParam = 0;
            mCfgDirChgLocalParam = 0;
            mCfgRelativePath = mUseRelativePath.Get();
        }

        if (GuiModal(XSTR(THPRAC_DIRECTORY_SETTING_MODAL))) {
            ImGui::PushTextWrapPos(LauncherWndGetSize().x * 0.9f);
            ImGui::Text(XSTR(THPRAC_DIRECTORY_SETTING_DESC));
            ImGui::NewLine();

            ImGui::RadioButton(XSTR(THPRAC_DIR_GLOBAL), &mCfgUseLocalDir, 0);
            ImGui::SameLine();
            ImGui::RadioButton((XSTR(THPRAC_DIR_LOCAL)), &mCfgUseLocalDir, 1);
            ImGui::SameLine();
            if (mCfgUseLocalDir == 1) {
                ImGui::Checkbox(XSTR(THPRAC_USE_REL_PATH), &mCfgRelativePath);
            } else {
                ImGui::BeginDisabled();
                ImGui::Checkbox(XSTR(THPRAC_USE_REL_PATH), &mCfgRelativePath);
                ImGui::EndDisabled();
            }

            if (gCfgIsLocalDir == mCfgUseLocalDir) {
                ImGui::BeginDisabled();
            }
            if (!gCfgIsLocalDir) {
                ImGui::Text(XSTR(THPRAC_EXISTING_GLOBAL_DATA));
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_IGNORE_GLOBAL_DATA), &mCfgDirChgGlobalParam, 0);
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_MOVE_TO_LOCAL), &mCfgDirChgGlobalParam, 1);
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_COPY_TO_LOCAL), &mCfgDirChgGlobalParam, 2);

                ImGui::Text(XSTR(THPRAC_EXISTING_LOCAL_DATA));
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_IGNORE_LOCAL_DATA), &mCfgDirChgLocalParam, 0);
                ImGui::SameLine();
                if (gCfgIsLocalDir != mCfgUseLocalDir && !gCfgLocalBackupAvaliable) {
                    ImGui::BeginDisabled();
                }
                ImGui::RadioButton(XSTR(THPRAC_USE_BACKUP_DATA), &mCfgDirChgLocalParam, 1);
                if (gCfgIsLocalDir != mCfgUseLocalDir && !gCfgLocalBackupAvaliable) {
                    ImGui::EndDisabled();
                }
                ImGui::SameLine();
                ImGui::BeginDisabled();
                ImGui::RadioButton(XSTR(THPRAC_OVERWRTITE_DATA), &mCfgDirChgLocalParam, 2);
                ImGui::EndDisabled();
                if (mCfgDirChgGlobalParam == 1 || mCfgDirChgGlobalParam == 2) {
                    mCfgDirChgLocalParam = 2;
                } else if (mCfgDirChgLocalParam == 2) {
                    mCfgDirChgLocalParam = 0;
                }
            } else {

                ImGui::Text(XSTR(THPRAC_EXISTING_GLOBAL_DATA));
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_IGNORE_GLOBAL_DATA), &mCfgDirChgGlobalParam, 0);
                ImGui::SameLine();
                if (gCfgIsLocalDir != mCfgUseLocalDir) {
                    ImGui::BeginDisabled();
                }
                ImGui::RadioButton(XSTR(THPRAC_OVERWRTITE_DATA), &mCfgDirChgGlobalParam, 1);
                if (gCfgIsLocalDir != mCfgUseLocalDir) {
                    ImGui::EndDisabled();
                }
                if (mCfgDirChgLocalParam == 1) {
                    mCfgDirChgGlobalParam = 1;
                } else if (mCfgDirChgGlobalParam == 1) {
                    mCfgDirChgGlobalParam = 0;
                }

                ImGui::Text(XSTR(THPRAC_EXISTING_LOCAL_DATA));
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_CREATE_BACKUP_DATA), &mCfgDirChgLocalParam, 0);
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_MOVE_TO_GLOBAL), &mCfgDirChgLocalParam, 1);
                ImGui::SameLine();
                ImGui::RadioButton(XSTR(THPRAC_PURGE_LOCAL), &mCfgDirChgLocalParam, 2);
            }
            if (gCfgIsLocalDir == mCfgUseLocalDir) {
                ImGui::EndDisabled();
            }

            if (gCfgIsLocalDir == mCfgUseLocalDir && mCfgRelativePath == mUseRelativePath.Get()) {
                if (GuiCornerButton(XSTR(THPRAC_CANCEL), nullptr, ImVec2(1.0f, 0.0f), true)) {
                    ImGui::CloseCurrentPopup();
                }
            } else {
                auto retnValue = GuiCornerButton(XSTR(THPRAC_APPLY), XSTR(THPRAC_CANCEL), ImVec2(1.0f, 0.0f), true);
                if (retnValue == 1) {
                    ImGui::CloseCurrentPopup();
                    confirmModalFlag = true;
                } else if (retnValue == 2) {
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::PopTextWrapPos();
            ImGui::EndPopup();
        }

        if (confirmModalFlag) {
            ImGui::OpenPopup(XSTR(THPRAC_DIRECTORY_CONFIRM));
        }
        if (GuiModal(XSTR(THPRAC_DIRECTORY_CONFIRM))) {
            ImGui::Text(XSTR(THPRAC_DIRECTORY_CONFIRM_DESC));
            auto buttonSize = ImGui::GetItemRectSize().x / 2.05f;
            if (ImGui::Button(XSTR(THPRAC_YES), ImVec2(buttonSize, 0))) {
                GuiLauncherMainTrigger(LAUNCHER_RESET);
                mCfgResetFlag = 2;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button(XSTR(THPRAC_NO), ImVec2(buttonSize, 0))) {
                dirSettingModalFlag = true;
                //ImGui::OpenPopup("Data directory##modal");
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (dirSettingModalFlag) {
            ImGui::OpenPopup(XSTR(THPRAC_DIRECTORY_SETTING_MODAL));
        }

        ImGui::SameLine();
        if (ImGui::Button(XSTR(THPRAC_DATADIR_OPEN))) {
            ShellExecuteW(NULL, L"open", LauncherGetDataDir().c_str(), NULL, NULL, SW_SHOW);
        }

        ImGui::SameLine();
        if (GuiButtonAndModalYesNo(XSTR(THPRAC_RESET_LAUNCHER), XSTR(THPRAC_RESET_LAUNCHER_MODAL), XSTR(THPRAC_RESET_LAUNCHER_WARNING), -1.0f, XSTR(THPRAC_YES), XSTR(THPRAC_NO))) {
            GuiLauncherMainTrigger(LAUNCHER_RESET);
            mCfgResetFlag = 1;
        }
    }

    void TextLink(const char* text, const wchar_t* link)
    {
        ImGui::Text(text);
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsMouseClicked(0)) {
                ShellExecuteW(NULL, NULL, link, NULL, NULL, SW_SHOW);
            }
        }
    }
    void GuiLicenceWnd()
    {
        if (ImGui::Button(XSTR(THPRAC_BACK))) {
            mGuiUpdFunc = [&]() { GuiMain(); };
        }
        ImGui::SameLine();
        GuiCenteredText("COPYING");
        ImGui::Separator();
        ImGui::BeginChild("COPYING_INFO");
        Gui::ShowLicenceInfo();
        ImGui::EndChild();
    }
    void GuiMain()
    {
        ImGui::Text(XSTR(THPRAC_LAUNCH_BEHAVIOR));
        ImGui::Separator();
        mAdminRights.Gui(XSTR(THPRAC_ADMIN_RIGHTS));

        ImGui::Text(XSTR(THPRAC_SETTING_LAUNCHER));
        ImGui::Separator();
        mCfgAfterLaunch.Gui(XSTR(THPRAC_AFTER_LAUNCH), XSTR(THPRAC_AFTER_LAUNCH_OPTION));
        mCfgFilterDefault.Gui(XSTR(THPRAC_FILTER_DEFAULT), XSTR(THPRAC_FILTER_DEFAULT_OPTION), XSTR(THPRAC_FILTER_DEFAULT_DESC));
        PathAndDirSettings();
        ImGui::NewLine();

        ImGui::Text(XSTR(THPRAC_GAME_ADJUSTMENTS));
        ImGui::Separator();
        mCfgUnlockRefreshRate.Gui(XSTR(THPRAC_UNLOCK_REFRESH_RATE), XSTR(THPRAC_UNLOCK_REFRESH_RATE_DESC));
        ImGui::NewLine();

        ImGui::Text(XSTR(THPRAC_SETTING_LANGUAGE));
        ImGui::Separator();
        mCfgLanguage.Gui(XSTR(THPRAC_LANGUAGE), u8"����\0English\0�ձ��Z\0\0");
        if (mOriginalLanguage != mCfgLanguage.Get()) {
            ImGui::Text(th_glossary_str[mCfgLanguage.Get()][THPRAC_LANGUAGE_HINT]);
        }
        ImGui::NewLine();

        ImGui::Text(XSTR(THPRAC_SETTING_ABOUT));
        ImGui::Separator();
        ImGui::Text(XSTR(TH_ABOUT_VERSION), GetVersionStr());
        ImGui::SameLine();
        if (ImGui::Button(XSTR(TH_ABOUT_SHOW_LICENCE))) {
            mGuiUpdFunc = [&]() { GuiLicenceWnd(); };
        }
    }

    THCfgCombo mCfgLanguage { "language", 0, 3 };
    THCfgCheckbox mCfgAlwaysOpen { "always_open_launcher", false };
    THCfgCombo mCfgAfterLaunch { "after_launch", 0, 3 };
    THCfgCombo mCfgFilterDefault { "filter_default", 0, 3 };
    THSetting<bool> mUseRelativePath { "use_relative_path", false };
    THCfgCheckbox mCfgUnlockRefreshRate { "unlock_refresh_rate", false };
    THCfgCheckbox mCfgCheckUpdate { "check_update", true };

    THCfgCheckbox mAdminRights { "thprac_admin_rights", false };
    THCfgCombo mCheckUpdateTiming { "check_update_timing", 0, 3 };
    THCfgCheckbox mUpdateWithoutConfirm { "update_without_confirmation", false };
    THCfgCombo mFilenameAfterUpdate { "filename_after_update", 0, 3 };
    int mOriginalLanguage;

    bool mGameTypeOpt[4];

    unsigned int mCfgResetFlag = 0;
    int mCfgUseLocalDir = 0;
    int mCfgDirChgGlobalParam = 0;
    int mCfgDirChgLocalParam = 0;
    bool mCfgRelativePath = false;

    std::function<void(void)> mGuiUpdFunc = []() {};
};

void LauncherCfgGuiUpd()
{
    THCfgGui::singleton().GuiUpdate();
}

void LauncherCfgReset()
{
    THCfgGui::singleton().LauncherCfgReset();
}

bool DeleteFileLoop(const wchar_t* fileName, size_t timeout = 0)
{
    auto attr = GetFileAttributesW(fileName);
    if (attr == INVALID_FILE_ATTRIBUTES || attr & FILE_ATTRIBUTE_DIRECTORY || attr & FILE_ATTRIBUTE_READONLY) {
        return false;
    }

    size_t localTimeout = 0;
    while (true) {
        if (DeleteFileW(fileName)) {
            return true;
        }
        Sleep(1000);
        localTimeout += 1000;
        if (localTimeout > timeout) {
            return false;
        }
    }
}
bool CheckMutexLoop(const wchar_t* mutex_name, size_t timeout = 0)
{
    size_t localTimeout = 0;
    while (true) {
        auto result = OpenMutexW(SYNCHRONIZE, FALSE, mutex_name);
        if (result) {
            CloseHandle(result);
            return true;
        }
        Sleep(500);
        localTimeout += 500;
        if (localTimeout > timeout) {
            return false;
        }
    }
}

}