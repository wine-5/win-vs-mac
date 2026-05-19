#include "Bios.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/input/KeyCode.h"
#include "core/utility/Color.h"

namespace
{
    enum class LineType { Normal, Ok, Warn, Bold, Dim, Head };

    struct BiosLine
    {
        const char* text;
        LineType    type { LineType::Normal };
        float       delay{};    // 直前の行からの遅延（秒）
    };

    constexpr BiosLine BIOS_LINES[] =
    {
        { "  AMI UEFI BIOS  -  WIN vs MAC Dungeon System v1.0.0  ",     LineType::Head,   0.00f },
        { "  Copyright (C) 2026  WIN vs MAC Development Team      ",    LineType::Head,   0.08f },
        { "",                                                            LineType::Normal, 0.14f },
        { "CPU  : WIN-CORE i9-X9900K @ 5.80GHz .......................... [OK]", LineType::Ok,   0.52f },
        { "Mem  : 32768 MB DDR5-6400 ..................................... [OK]", LineType::Ok,   0.24f },
        { "",                                                            LineType::Normal, 0.08f },
        { "Detecting storage devices...",                                LineType::Normal, 0.32f },
        { "  C:\\ NTFS  512 GB  (Windows OS + Dungeon Core) .......... [OK]", LineType::Ok,   0.21f },
        { "  D:\\ NTFS    2 TB  (User Data + Enemy Database) ......... [OK]", LineType::Ok,   0.21f },
        { "  Z:\\ ???   ??? GB  (UNKNOWN - Suspicious Files) ... [WARN]", LineType::Warn, 0.48f },
        { "",                                                            LineType::Normal, 0.08f },
        { "PCI-E Devices:",                                             LineType::Normal, 0.18f },
        { "  GPU : NVIDIA GeForce RTX 4090 ............................... [OK]", LineType::Ok, 0.17f },
        { "  NET : Intel AX210 WiFi 6E ................................... [OK]", LineType::Ok, 0.17f },
        { "",                                                            LineType::Normal, 0.08f },
        { "DUNGEON SUBSYSTEM INITIALIZING...",                           LineType::Bold,   0.36f },
        { "  Kernel Modules      [############] 100% ................. [OK]", LineType::Ok, 0.31f },
        { "  Enemy Database      [############] 100% ................. [OK]", LineType::Ok, 0.27f },
        { "  Physics Engine      [############] 100% ................. [OK]", LineType::Ok, 0.29f },
        { "  Dungeon Generator   [############] 100% ................. [OK]", LineType::Ok, 0.37f },
        { "",                                                            LineType::Normal, 0.10f },
        { "[!] WARNING: Unauthorized process detected  (mac_invasion.exe)", LineType::Warn, 0.68f },
        { "    Countermeasures loading...",                              LineType::Warn,   0.40f },
        { "",                                                            LineType::Normal, 0.20f },
        { "System Ready.  Starting WIN vs MAC...",                       LineType::Bold,   0.56f },
        { "",                                                            LineType::Normal, 0.08f },
        { "----------------------------------------------------------------------", LineType::Dim, 0.08f },
        { " [F2] Enter Setup   |   [F12] Boot Menu   |   [ESC] Skip ", LineType::Dim,  0.05f },
    };

    constexpr int LINE_COUNT{ static_cast<int>(sizeof(BIOS_LINES) / sizeof(BIOS_LINES[0])) };

    constexpr float BIOS_FONT_RATIO        = 16.0f / 720.0f;
    constexpr float BIOS_LINE_HEIGHT_RATIO = 24.0f / 720.0f;
    constexpr float BIOS_PADDING_X_RATIO   = 36.0f / 1280.0f;
    constexpr float BIOS_PADDING_Y_RATIO   = 20.0f / 720.0f;

    unsigned int lineTypeToColor(LineType type) noexcept
    {
        using core::utility::Color;
        switch (type)
        {
        case LineType::Ok:   return Color::rgb(  0, 200,   0);
        case LineType::Warn: return Color::rgb(200, 200,   0);
        case LineType::Bold: return Color::WHITE;
        case LineType::Dim:  return Color::rgb( 96,  96,  96);
        case LineType::Head: return Color::YELLOW;
        default:             return Color::LIGHT_GRAY;
        }
    }
}

namespace game::scene
{
    Bios::Bios(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_inputProvider{ inputProvider }
        , m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
    {
        // 各行の表示タイミング（累積時間）を事前計算
        m_lineTimestamps.reserve(LINE_COUNT);
        float acc{};
        for (int i{}; i < LINE_COUNT; ++i)
        {
            acc += BIOS_LINES[i].delay;
            m_lineTimestamps.push_back(acc);
        }
    }

    void Bios::update(float deltaTime)
    {
        if (m_transitioning) return;

        // ESC で即時スキップ
        if (m_inputProvider.isKeyPressed(core::input::KeyCode::Escape))
        {
            transitionToLockscreen();
            return;
        }

        m_elapsed += deltaTime;

        // 可視行数を更新
        while (m_visibleCount < LINE_COUNT &&
            m_elapsed >= m_lineTimestamps[m_visibleCount])
        {
            ++m_visibleCount;
        }

        // 全行表示済み → 自動遷移タイマー
        if (m_visibleCount >= LINE_COUNT)
        {
            m_postAllElapsed += deltaTime;
            if (m_postAllElapsed >= AUTO_ADVANCE_DELAY)
                transitionToLockscreen();
        }
    }

    void Bios::draw()
    {
        const int screenW   { m_screen.getWidth() };
        const int screenH   { m_screen.getHeight() };
        const int fontSize  { static_cast<int>(screenH * BIOS_FONT_RATIO) };
        const int lineHeight{ static_cast<int>(screenH * BIOS_LINE_HEIGHT_RATIO) };
        const int padX      { static_cast<int>(screenW * BIOS_PADDING_X_RATIO) };
        const int padY      { static_cast<int>(screenH * BIOS_PADDING_Y_RATIO) };

        // 黒背景を明示的に塗りつぶす
        m_uiRenderer.drawBox(0, 0, screenW, screenH,
            core::utility::Color::rgb(0, 0, 0), true);

        for (int i{}; i < m_visibleCount; ++i)
        {
            const auto& line { BIOS_LINES[i] };
            const int   y    { padY + i * lineHeight };
            const auto  color{ lineTypeToColor(line.type) };

            // Head タイプは青背景を先に描画
            if (line.type == LineType::Head)
                m_uiRenderer.drawBox(padX, y, screenW - 2 * padX, lineHeight,
                    core::utility::Color::rgb(0, 0, 170), true);

            if (line.text[0] != '\0')
                m_uiRenderer.drawText(padX, y, line.text, color, fontSize);
        }
    }

    void Bios::transitionToLockscreen() noexcept
    {
        m_transitioning = true;
        auto* sm{ core::base::ServiceLocator::get<SceneManager>() };
        if (sm) sm->changeScene(SceneType::Lockscreen);
    }
}
