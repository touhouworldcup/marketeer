#pragma once

#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#define NOMINMAX
#include <Windows.h>
#include <imgui.h>
#include <string>
#include <functional>
#include <vector>
#include <random>
#include <ctime>
#include <imgui.h>
#pragma warning(disable : 4091)
#include <Shlobj.h>
#pragma warning(default : 4091)
#include "thprac_launcher_wnd.h"
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

namespace THPrac {
#pragma region Locale
static void* _str_cvt_buffer(size_t size)
{
    static size_t bufferSize = 512;
    static void* bufferPtr = nullptr;
    if (!bufferPtr) {
        bufferPtr = malloc(bufferSize);
    }
    if (bufferSize < size) {
        for (; bufferSize < size; bufferSize *= 2)
            ;
        if (bufferPtr) {
            free(bufferPtr);
        }
        bufferPtr = malloc(size);
    }
    return bufferPtr;
}
static std::string utf16_to_utf8(const std::wstring& utf16)
{
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, nullptr, 0, NULL, NULL);
    char* utf8 = (char*)_str_cvt_buffer(utf8Length);
    WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, utf8, utf8Length, NULL, NULL);
    return std::string(utf8);
}
static std::string utf16_to_utf8(const wchar_t* utf16)
{
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, nullptr, 0, NULL, NULL);
    char* utf8 = (char*)_str_cvt_buffer(utf8Length);
    WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, utf8Length, NULL, NULL);
    return std::string(utf8);
}
static std::wstring utf8_to_utf16(const std::string& utf8)
{
    int utf16Length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    wchar_t* utf16 = (wchar_t*)_str_cvt_buffer(utf16Length);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, utf16, utf16Length);
    return std::wstring(utf16);
}
static std::wstring utf8_to_utf16(const char* utf8)
{
    int utf16Length = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    wchar_t* utf16 = (wchar_t*)_str_cvt_buffer(utf16Length);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16Length);
    return std::wstring(utf16);
}
static std::string utf16_to_mb(const std::wstring& utf16)
{
    int utf8Length = WideCharToMultiByte(CP_ACP, 0, utf16.c_str(), -1, nullptr, 0, NULL, NULL);
    char* utf8 = (char*)_str_cvt_buffer(utf8Length);
    WideCharToMultiByte(CP_ACP, 0, utf16.c_str(), -1, utf8, utf8Length, NULL, NULL);
    return std::string(utf8);
}
static std::string utf16_to_mb(const wchar_t* utf16)
{
    int utf8Length = WideCharToMultiByte(CP_ACP, 0, utf16, -1, nullptr, 0, NULL, NULL);
    char* utf8 = (char*)_str_cvt_buffer(utf8Length);
    WideCharToMultiByte(CP_ACP, 0, utf16, -1, utf8, utf8Length, NULL, NULL);
    return std::string(utf8);
}
static std::wstring mb_to_utf16(const std::string& utf8)
{
    int utf16Length = MultiByteToWideChar(CP_ACP, 0, utf8.c_str(), -1, nullptr, 0);
    wchar_t* utf16 = (wchar_t*)_str_cvt_buffer(utf16Length);
    MultiByteToWideChar(CP_ACP, 0, utf8.c_str(), -1, utf16, utf16Length);
    return std::wstring(utf16);
}
static std::wstring mb_to_utf16(const char* utf8)
{
    int utf16Length = MultiByteToWideChar(CP_ACP, 0, utf8, -1, nullptr, 0);
    wchar_t* utf16 = (wchar_t*)_str_cvt_buffer(utf16Length);
    MultiByteToWideChar(CP_ACP, 0, utf8, -1, utf16, utf16Length);
    return std::wstring(utf16);
}
#pragma endregion

#define JsonAddMember(json, key, value, alloc) json.AddMember(rapidjson::Value(key, alloc).Move(), rapidjson::Value(value).Move(), alloc);
#define JsonAddMemberA(json, key, value, alloc) json.AddMember(rapidjson::Value(key, alloc).Move(), rapidjson::Value(value, alloc).Move(), alloc);

class GuiThread {
public:
    GuiThread() = default;
    GuiThread(const GuiThread&) = delete;
    GuiThread& operator=(GuiThread&) = delete;
    GuiThread(GuiThread&&) = delete;
    GuiThread& operator=(GuiThread&&) = delete; 
    GuiThread(LPTHREAD_START_ROUTINE threadFunc)
    {
        mThreadFunc = threadFunc;
    }

    DWORD GetExitCode()
    {
        if (mThreadHnd != INVALID_HANDLE_VALUE) {
            DWORD exitCode;
            GetExitCodeThread(mThreadHnd, &exitCode);
            return exitCode;
        }
        return 0;
    }
    bool Start(void* threadData = nullptr)
    {
        if (mThreadHnd == INVALID_HANDLE_VALUE) {
            mThreadHnd = CreateThread(NULL, 0, mThreadFunc, threadData, 0, NULL);
            return mThreadHnd != INVALID_HANDLE_VALUE;
        }
        return false;
    }
    bool Stop()
    {
        if (mThreadHnd != INVALID_HANDLE_VALUE) {
            if (IsActive()) {
                TerminateThread(mThreadHnd, 0);
            }
            CloseHandle(mThreadHnd);
            mThreadHnd = INVALID_HANDLE_VALUE;
        }
        return true;
    }
    bool Wait()
    {
        if (mThreadHnd != INVALID_HANDLE_VALUE) {
            WaitForSingleObject(mThreadHnd, INFINITE);
            return true;
        }
        return false;
    }
    bool IsActive()
    {
        return GetExitCode() == STILL_ACTIVE;
    }
    void SetThreadFunc(LPTHREAD_START_ROUTINE threadFunc)
    {
        mThreadFunc = threadFunc;
    }

private:
    HANDLE mThreadHnd = INVALID_HANDLE_VALUE;
    LPTHREAD_START_ROUTINE mThreadFunc = nullptr;
};

class GuiWaitingAnm {
public:
    GuiWaitingAnm() = default;
    
    std::string Get()
    {
        mCounter++;
        if (mCounter >= 240) {
            mCounter = 0;
        }
        switch (mCounter) {
        case 0:
            mAscii = ".";
            break;
        case 80:
            mAscii = "..";
            break;
        case 160:
            mAscii = "...";
            break;
        default:
            break;
        }
        return mAscii;
    }
    void Reset()
    {
        mCounter = 0;
        mAscii = ".";
    }

private:
    int mCounter = 0;
    std::string mAscii = ".";
};

static void DeleteFolder(std::string path)
{
    path += '\0';
    path += '\0';
    SHFILEOPSTRUCTA fileOp;
    memset(&fileOp, 0, sizeof(SHFILEOPSTRUCTW));
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = path.c_str();
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR | FOF_SILENT;
    SHFileOperationA(&fileOp);
}

static void DeleteFolder(std::wstring path)
{
    path += L'\0';
    path += L'\0';
    SHFILEOPSTRUCTW fileOp;
    memset(&fileOp, 0, sizeof(SHFILEOPSTRUCTW));
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = path.c_str();
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR | FOF_SILENT;
    SHFileOperationW(&fileOp);
}

static void MovWndToTop(HWND m_hWnd)
{
    HWND hCurWnd = ::GetForegroundWindow();
    DWORD dwMyID = ::GetCurrentThreadId();
    DWORD dwCurID = ::GetWindowThreadProcessId(hCurWnd, NULL);
    ::AttachThreadInput(dwCurID, dwMyID, TRUE);
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    ::SetForegroundWindow(m_hWnd);
    ::SetFocus(m_hWnd);
    ::SetActiveWindow(m_hWnd);
    ::AttachThreadInput(dwCurID, dwMyID, FALSE);
}

template<typename T>
static std::function<T(void)> GetRndGenerator(T min, T max, std::mt19937::result_type seed = 0)
{
    //std::mt19937::result_type seed = time(0);
    if (!seed) {
        seed = (std::mt19937::result_type)time(0);
        //std::random_device rd;
        //seed = rd();
    }
    auto dice_rand = std::bind(std::uniform_int_distribution<T>(min, max), std::mt19937(seed));
    return dice_rand;
}

static std::string GetSuffixFromPath(const char* pathC)
{
    std::string path = pathC;
    auto pos = path.rfind('.');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return std::string("");
}

static std::string GetSuffixFromPath(std::string& path) 
{
    auto pos = path.rfind('.');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return std::string("");
}

static std::string GetDirFromFullPath(std::string& dir)
{
    auto slashPos = dir.rfind('\\');
    if (slashPos == std::string::npos) {
        slashPos = dir.rfind('/');
    }
    if (slashPos == std::string::npos) {
        return dir;
    }
    return dir.substr(0, slashPos + 1);
}

static std::wstring GetDirFromFullPath(std::wstring& dir)
{
    auto slashPos = dir.rfind(L'\\');
    if (slashPos == std::wstring::npos) {
        slashPos = dir.rfind(L'/');
    }
    if (slashPos == std::wstring::npos) {
        return dir;
    }
    return dir.substr(0, slashPos + 1);
}

static std::string GetNameFromFullPath(std::string& dir)
{
    auto slashPos = dir.rfind('\\');
    if (slashPos == std::string::npos) {
        slashPos = dir.rfind('/');
    }
    if (slashPos == std::string::npos) {
        return dir;
    }
    return dir.substr(slashPos + 1);
}

static std::wstring GetNameFromFullPath(std::wstring& dir)
{
    auto slashPos = dir.rfind(L'\\');
    if (slashPos == std::wstring::npos) {
        slashPos = dir.rfind(L'/');
    }
    if (slashPos == std::wstring::npos) {
        return dir;
    }
    return dir.substr(slashPos + 1);
}

static std::string GetCleanedPath(std::string& path)
{
    std::string result;
    wchar_t lastChar = '\0';
    for (auto& c : path) {
        if (c == '/' || c == '\\') {
            if (lastChar == '\\') {
                continue;
            } else {
                result.push_back('\\');
                lastChar = '\\';
            }
        } else {
            result.push_back(c);
            lastChar = c;
        }
    }
    return result;
}

static std::wstring GetCleanedPath(std::wstring& path)
{
    std::wstring result;
    wchar_t lastChar = '\0';
    for (auto& c : path) {
        if (c == L'/' || c == L'\\') {
            if (lastChar == L'\\') {
                continue;
            } else {
                result.push_back(L'\\');
                lastChar = L'\\';
            }
        } else {
            result.push_back(c);
            lastChar = c;
        }
    }
    return result;
}

static std::string GetUnifiedPath(std::string& path)
{
    std::string result;
    wchar_t lastChar = '\0';
    for (auto& c : path) {
        if (c == '/' || c == '\\') {
            if (lastChar == '\\') {
                continue;
            } else {
                result.push_back('\\');
                lastChar = '\\';
            }
        } else {
            auto lower = tolower(c);
            result.push_back(lower);
            lastChar = lower;
        }
    }
    return result;
}

static std::wstring GetUnifiedPath(std::wstring& path) 
{
    std::wstring result;
    wchar_t lastChar = '\0';
    for (auto& c : path) {
        if (c == L'/' || c == L'\\') {
            if (lastChar == L'\\') {
                continue;
            } else {
                result.push_back(L'\\');
                lastChar = L'\\';
            }
        } else {
            auto lower = towlower(c);
            result.push_back(lower);
            lastChar = lower;
        }
    }
    return result;
}

static std::vector<ImVec2> g_guiCursorStack;
static void GuiPushCursorPos()
{
    g_guiCursorStack.push_back(ImGui::GetCursorPos());
}
static void GuiPopCursorPos()
{
    ImGui::SetCursorPos(g_guiCursorStack.back());
    g_guiCursorStack.pop_back();
}

static void GuiColumnText(const char* text)
{
    auto columnX = ImGui::GetColumnWidth();
    auto itemX = ImGui::CalcTextSize(text).x + ImGui::GetStyle().ItemSpacing.x;
    ImGui::Text(text);
    if (ImGui::IsItemHovered()) {
        if (itemX > columnX) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetIO().DisplaySize.x * 0.9f);
            ImGui::Text(text);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

static void GuiCenteredText(const char* text)
{
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(text, 0, false, 0.0f).x) / 2.0f);
    ImGui::TextWrapped(text);
}

static void GuiSetPosXText(const char* text, float offset = 0.0f)
{
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(text, 0, false, 0.0f).x) / 2.0f - offset);
}

static void GuiSetPosYRel(float rel)
{
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * rel);
}

static void GuiHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void GuiHelpMarkerEx(const char* tiptxt, const char* desc)
{
    ImGui::TextDisabled(tiptxt);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void GuiDescMarker(const char* descTxt)
{
    auto xNow = ImGui::GetCursorPosX();
    auto xWnd = ImGui::GetWindowSize().x;
    auto xTxt = ImGui::CalcTextSize(descTxt).x + ImGui::GetStyle().ItemSpacing.x;
    ImGui::TextDisabled(descTxt);
    if (ImGui::IsItemHovered()) {
        if (xTxt > (xWnd - xNow)) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(xWnd * 0.9f);
            ImGui::TextUnformatted(descTxt);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

static int GuiCornerButton(const char* text, const char* text2 = nullptr, ImVec2& offset = ImVec2(1.5f, 0.5f), bool useCurrentY = false)
{
    int result = 0;
    auto& style = ImGui::GetStyle();
    auto currentCursorY = ImGui::GetCursorPosY();
    ImVec2 cursor = ImGui::GetWindowSize();
    auto textSize = ImGui::CalcTextSize(text);
    cursor.x = cursor.x - textSize.x - offset.x * ImGui::GetFontSize();
    cursor.y = cursor.y - textSize.y - offset.y * ImGui::GetFontSize();
    if (cursor.y < currentCursorY || useCurrentY) {
        cursor.y = currentCursorY;
    }
    if (text2) {
        cursor.x -= style.FramePadding.x * 2 + style.ItemSpacing.x;
        cursor.x -= ImGui::CalcTextSize(text2).x;
    }

    ImGui::SetCursorPos(cursor);
    if (ImGui::Button(text)) {
        result = 1;
    }
    if (text2) {
        ImGui::SameLine();
        if (ImGui::Button(text2)) {
            result = 2;
        }
    }

    return result;
}

static bool GuiButtonRelCentered(const char* buttonText, float posYRel, ImVec2& sizeRel)
{
    auto wndSize = ImGui::GetWindowSize();
    ImGui::SetCursorPosX((wndSize.x - wndSize.x * sizeRel.x) / 2.0f);
    ImGui::SetCursorPosY(wndSize.y * posYRel);
    return ImGui::Button(buttonText, ImVec2(wndSize.x * sizeRel.x, wndSize.y * sizeRel.y));
}

static bool GuiButtonTxtCentered(const char* buttonText, float posYRel)
{
    auto wndSize = ImGui::GetWindowSize();
    auto textSize = ImGui::CalcTextSize(buttonText);
    ImGui::SetCursorPosX((wndSize.x - textSize.x) / 2.0f);
    ImGui::SetCursorPosY(wndSize.y * posYRel);
    return ImGui::Button(buttonText);
}

static bool GuiButtonTxtCentered(const char* buttonText)
{
    auto wndSize = ImGui::GetWindowSize();
    auto textSize = ImGui::CalcTextSize(buttonText);
    ImGui::SetCursorPosX((wndSize.x - textSize.x) / 2.0f);
    //ImGui::SetCursorPosY(wndSize.y * posYRel);
    return ImGui::Button(buttonText);
}

static bool GuiModal(const char* modalTitle, ImVec2 sizeRel = ImVec2(0.0f, 0.0f))
{
    auto wndSize = LauncherWndGetSize();
    wndSize.x *= 0.5f;
    wndSize.y *= 0.5f;
    ImGui::SetNextWindowPos(wndSize, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (sizeRel.x != 0.0f || sizeRel.y != 0.0f) {
        ImGui::SetNextWindowSize(sizeRel, ImGuiCond_Always);
    }
    return ImGui::BeginPopupModal(modalTitle, NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
}

static bool GuiModalFullScreen(const char* modalTitle)
{
    auto wndSize = LauncherWndGetSize();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(wndSize, ImGuiCond_Appearing);
    return ImGui::BeginPopupModal(modalTitle, NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
}

static bool GuiModalOkCancel(const char* modalTitle, const char* modalText, float buttonSize = 6.0f)
{
    bool result = false;
    if (GuiModal(modalTitle)) {
        ImGui::PushTextWrapPos(ImGui::GetIO().DisplaySize.x * 0.9f);
        ImGui::TextWrapped(modalText);
        ImGui::PopTextWrapPos();
        if (ImGui::Button("OK", ImVec2(buttonSize * ImGui::GetFontSize(), 0))) {
            ImGui::CloseCurrentPopup();
            result = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonSize * ImGui::GetFontSize(), 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return result;
}

static bool GuiButtonModal(const char* buttonText, const char* modalTitle)
{
    if (ImGui::Button(buttonText)) {
        ImGui::OpenPopup(modalTitle);
        return true;
    }
    return false;
}

static bool GuiButtonYesNo(const char* buttonText1 = "OK", const char* buttonText2 = "Cancel", float buttonSize = -1.0f)
{
    if (buttonSize == -1.0f) {
        buttonSize = ImGui::GetItemRectSize().x / 2.05f;
    } else {
        buttonSize = buttonSize * ImGui::GetFontSize();
    }

    if (ImGui::Button(buttonText1, ImVec2(buttonSize, 0))) {
        ImGui::CloseCurrentPopup();
        return true;
    }
    ImGui::SameLine();
    if (ImGui::Button(buttonText2, ImVec2(buttonSize, 0))) {
        ImGui::CloseCurrentPopup();
    }
    return false;
}

static bool GuiButtonAndModalYesNo(const char* buttonText, const char* modalTitle, const char* modalText, float buttonSize = 6.0f, const char* buttonText1 = "OK", const char* buttonText2 = "Cancel")
{
    bool result = false;

    GuiButtonModal(buttonText, modalTitle);
    if (GuiModal(modalTitle)) {
        ImGui::Text(modalText);
        if (GuiButtonYesNo(buttonText1, buttonText2, buttonSize)) {
            result = true;
        }
        ImGui::EndPopup();
    }

    return result;
}

}