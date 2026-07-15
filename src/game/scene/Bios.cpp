#include "Bios.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/input/KeyCode.h"
#include "core/utility/Color.h"

namespace
{
    enum class LineType { Content, Category, Header, Footer, Guide };

    struct BiosLine
    {
        const char* m_text;
        LineType    m_type { LineType::Content };
        float       m_delay{};    // 直前の行からの遅延（秒）
    };

    // BIOS 起動シーケンスの表示行。各行は順序通りに画面に表示される
    // 形式: { テキスト, 行タイプ, 前の行からの遅延時間（秒） }
    // 遅延時間をマジックナンバーである理由として定数化にしても再利用する予定がなく、冗長になってしまうだけだと思ったため
    constexpr BiosLine BIOS_LINES[] =
    {
        { "  [ESC]: Skip  ",                                                        LineType::Guide,    0.00f },
        { "  AMI UEFI BIOS  -  WIN vs MAC.exe v1.0.0  ",                            LineType::Header,   0.00f },
        { "  Copyright (C) 2026  WIN vs MAC wine-5      ",                          LineType::Header,   0.08f },
        { "----------------------------------------------------------------------", LineType::Category, 0.08f },
        { "Loading Core Systems",                                                   LineType::Category, 0.32f },
        { "  Service Locator ........................................ [OK]",        LineType::Content,  0.21f },
        { "  ECS Entity Manager ..................................... [OK]",        LineType::Content,  0.21f },
        { "  Layered Architecture ................................... [OK]",        LineType::Content,  0.18f },
        { "  Singleton Pattern ....................................... [OK]",       LineType::Content,  0.17f },
        { "  EventBus ............................................... [OK]",        LineType::Content,  0.17f },
        { "  Chain of Responsibility ................................. [OK]",       LineType::Content,  0.18f },
        { "----------------------------------------------------------------------", LineType::Category, 0.08f },
        { "External Libraries",                                                     LineType::Category, 0.24f },
        { "  DxLib 3D Graphics ....................................... [OK]",       LineType::Content,  0.27f },
        { "  Raylib Graphics Engine ................................... [OK]",      LineType::Content,  0.21f },
        { "  RapidJSON Parser ......................................... [OK]",      LineType::Content,  0.29f },
        { "  Effekseer Particle Engine ................................ [OK]",      LineType::Content,  0.31f },
        { "  WebView2 Integration ..................................... [OK]",      LineType::Content,  0.21f },
        { "----------------------------------------------------------------------", LineType::Category, 0.08f },
        { "Programming Language",                                                   LineType::Category, 0.24f },
        { "  C++17 Standard .......................................... [OK]",       LineType::Content,  0.21f },
        { "  GLSL/HLSL Shaders ....................................... [OK]",       LineType::Content,  0.17f },
        { "  HTML/CSS/JavaScript ..................................... [OK]",       LineType::Content,  0.18f },
        { "  JSON Data Format ......................................... [OK]",      LineType::Content,  0.17f },
        { "System Ready.  Starting WIN vs MAC.exe...",                              LineType::Content,  0.56f },
        { "[ESC] Skip ",                                                            LineType::Footer,   0.5f },
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
        case LineType::Content:  return Color::DARK_BLUE;
        case LineType::Category:  return Color::BLACK;
        case LineType::Header:   return Color::WHITE;
        case LineType::Footer:   return Color::WHITE;
        case LineType::Guide:    return Color::BLACK;
        default:                 return Color::BLACK;
        }
    }
} // namespace

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
            acc += BIOS_LINES[i].m_delay;
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

        // 白背景を明示的に塗りつぶす
        m_uiRenderer.drawBox(0, 0, screenW, screenH,
            core::utility::Color::WHITE, true);

        for (int i{}; i < m_visibleCount; ++i)
        {
            const auto& line { BIOS_LINES[i] };
            const int   y    { padY + i * lineHeight };
            const auto  color{ lineTypeToColor(line.m_type) };

            if (line.m_type == LineType::Guide)
                m_uiRenderer.drawBox(0, y, screenW, lineHeight,
                    core::utility::Color::MEDIUM_GREEN, true);
            else if (line.m_type == LineType::Header || line.m_type == LineType::Footer)
                m_uiRenderer.drawBox(0, y, screenW, lineHeight,
                    core::utility::Color::BLUE, true);

            if (line.m_text[0] != '\0')
                m_uiRenderer.drawText(padX, y, line.m_text, color, fontSize);
        }
    }

    void Bios::transitionToLockscreen() noexcept
    {
        m_transitioning = true;
        auto* sm{ core::base::ServiceLocator::get<SceneManager>() };
        if (sm) sm->changeScene(SceneType::Lockscreen);
    }
} // namespace game::scene
