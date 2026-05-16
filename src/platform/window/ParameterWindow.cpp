#include <windows.h>
#include <sstream>
#include "ParameterWindow.h"
#include "core/data/JobInfo.h"
#include "core/interface/ILogger.h"

namespace platform::window
{
    ParameterWindow::ParameterWindow(int x, int y, int width, int height) noexcept
        : WindowBase(L"ParameterWindowClass", L"Status Display", x, y, width, height)
    {
    }

    void ParameterWindow::refresh(const core::data::JobInfo& jobInfo) noexcept
    {
        // ウィンドウが作成されていない場合は何もしない
        if (!isCreated()) return;

        // SetWindowTextW: STATIC コントロールのテキストを更新
        // ウィンドウハンドルと表示する文字列を指定

        // 名前（std::stringをstd::wstringに変換して表示）
        std::wstring nameStr(jobInfo.m_name.begin(), jobInfo.m_name.end());
        SetWindowTextW(m_nameValue, nameStr.c_str());

        // スキル名
        std::wstring skillStr(jobInfo.m_skillName.begin(), jobInfo.m_skillName.end());
        SetWindowTextW(m_skillValue, skillStr.c_str());

        // HP（float を int に変換して文字列化）
        std::wstringstream hpStream;
        hpStream << static_cast<int>(jobInfo.m_hp);
        SetWindowTextW(m_hpValue, hpStream.str().c_str());

        // ATK
        std::wstringstream atkStream;
        atkStream << static_cast<int>(jobInfo.m_atk);
        SetWindowTextW(m_atkValue, atkStream.str().c_str());

        // DEF
        std::wstringstream defStream;
        defStream << static_cast<int>(jobInfo.m_def);
        SetWindowTextW(m_defValue, defStream.str().c_str());

        // SPD
        std::wstringstream spdStream;
        spdStream << static_cast<int>(jobInfo.m_spd);
        SetWindowTextW(m_spdValue, spdStream.str().c_str());
    }

    void ParameterWindow::onCreateControls(HWND hwnd)
    {
        RECT clientRect{};
        GetClientRect(hwnd, &clientRect);
        int windowWidth{ clientRect.right - clientRect.left };
        int windowHeight{ clientRect.bottom - clientRect.top };

        int labelWidth{ windowWidth * 40 / 100 };
        int valueWidth{ windowWidth * 50 / 100 };
        int height{ windowHeight * 12 / 100 };
        int startY{ windowHeight * 5 / 100 };
        int spacing{ windowHeight * 18 / 100 };
        int startX{ windowWidth * 5 / 100 };

        // Name ラベル作成（"Name:"という文字列を表示する読み取り専用のSTATICコントロール）
        // STATIC: テキストラベル用のコントロール（編集不可）
        m_nameLabel = CreateWindowW(
            L"STATIC", L"Name:", WS_VISIBLE | WS_CHILD,
            startX, startY, labelWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        LOG(m_nameLabel ? "[ParameterWindow] Name label created successfully" : "[ParameterWindow] Name label FAILED to create");

        // Name 値表示（空文字列で初期化、後で refresh() で更新）
        // SS_LEFT: テキスト左寄せ
        m_nameValue = CreateWindowW(
            L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_LEFT,
            startX + labelWidth, startY, valueWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_nameValue ? L"[ParameterWindow] Name value created successfully\n" : L"[ParameterWindow] Name value FAILED to create\n");

        // Skill ラベルと値
        m_skillLabel = CreateWindowW(
            L"STATIC", L"Skill:", WS_VISIBLE | WS_CHILD,
            startX, startY + spacing, labelWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_skillLabel ? L"[ParameterWindow] Skill label created successfully\n" : L"[ParameterWindow] Skill label FAILED to create\n");

        m_skillValue = CreateWindowW(
            L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_LEFT,
            startX + labelWidth, startY + spacing, valueWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_skillValue ? L"[ParameterWindow] Skill value created successfully\n" : L"[ParameterWindow] Skill value FAILED to create\n");

        // HP ラベルと値（SS_RIGHT: 右寄せで数値表示）
        m_hpLabel = CreateWindowW(
            L"STATIC", L"HP:", WS_VISIBLE | WS_CHILD,
            startX, startY + spacing * 2, labelWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_hpLabel ? L"[ParameterWindow] HP label created successfully\n" : L"[ParameterWindow] HP label FAILED to create\n");

        m_hpValue = CreateWindowW(
            L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_RIGHT,
            startX + labelWidth, startY + spacing * 2, valueWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_hpValue ? L"[ParameterWindow] HP value created successfully\n" : L"[ParameterWindow] HP value FAILED to create\n");

        // ATK ラベルと値
        m_atkLabel = CreateWindowW(
            L"STATIC", L"ATK:", WS_VISIBLE | WS_CHILD,
            startX, startY + spacing * 3, labelWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_atkLabel ? L"[ParameterWindow] ATK label created successfully\n" : L"[ParameterWindow] ATK label FAILED to create\n");

        m_atkValue = CreateWindowW(
            L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_RIGHT,
            startX + labelWidth, startY + spacing * 3, valueWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_atkValue ? L"[ParameterWindow] ATK value created successfully\n" : L"[ParameterWindow] ATK value FAILED to create\n");

        // DEF ラベルと値
        m_defLabel = CreateWindowW(
            L"STATIC", L"DEF:", WS_VISIBLE | WS_CHILD,
            startX, startY + spacing * 4, labelWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_defLabel ? L"[ParameterWindow] DEF label created successfully\n" : L"[ParameterWindow] DEF label FAILED to create\n");

        m_defValue = CreateWindowW(
            L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_RIGHT,
            startX + labelWidth, startY + spacing * 4, valueWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_defValue ? L"[ParameterWindow] DEF value created successfully\n" : L"[ParameterWindow] DEF value FAILED to create\n");

        // SPD ラベルと値
        m_spdLabel = CreateWindowW(
            L"STATIC", L"SPD:", WS_VISIBLE | WS_CHILD,
            startX, startY + spacing * 5, labelWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_spdLabel ? L"[ParameterWindow] SPD label created successfully\n" : L"[ParameterWindow] SPD label FAILED to create\n");

        m_spdValue = CreateWindowW(
            L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_RIGHT,
            startX + labelWidth, startY + spacing * 5, valueWidth, height,
            hwnd, nullptr, GetModuleHandleW(nullptr), nullptr
        );
        OutputDebugStringW(m_spdValue ? L"[ParameterWindow] SPD value created successfully\n" : L"[ParameterWindow] SPD value FAILED to create\n");
    }
}
