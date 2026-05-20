#include "Loading.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/utility/Color.h"
#include "core/base/ServiceLocator.h"
#include "platform/window/loading/LoadingWindow.h"

namespace game::scene
{
    Loading::Loading(core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
    {
        // DxLib のクライアント領域を取得
        HWND dxlibHwnd = static_cast<HWND>(m_screen.getNativeWindowHandle());

        RECT clientRect{};
        GetClientRect(dxlibHwnd, &clientRect);
        int screenWidth{ clientRect.right };
        int screenHeight{ clientRect.bottom };

        POINT origin{ 0, 0 };
        ClientToScreen(dxlibHwnd, &origin);
        int originX{ origin.x };
        int originY{ origin.y };

        // ローディングウィンドウを作成
        // スクリーン中央に配置（スクリーンサイズの80%）
        int windowWidth = screenWidth * 80 / 100;
        int windowHeight = screenHeight * 80 / 100;
        int windowX = originX + (screenWidth - windowWidth) / 2;
        int windowY = originY + (screenHeight - windowHeight) / 2;

        m_loadingWindow = std::make_unique<platform::window::loading::LoadingWindow>(
            windowX, windowY, windowWidth, windowHeight);

        // ローディング完了時のコールバックを設定
        m_loadingWindow->setOnLoadingComplete([this]() {
            this->notifyLoadingComplete();
        });

        // ウィンドウを作成・表示
        if (m_loadingWindow->create())
            m_loadingWindow->show();
    }

    Loading::~Loading() noexcept
    {
        if (m_loadingWindow)
            m_loadingWindow->destroy();
    }

    void Loading::update(float deltaTime)
    {
        switch (m_state)
        {
        case State::FadeIn:
            m_state = State::Loading;
            break;

        case State::Loading:
            // ローディング画面の完了を待つ
            break;

        case State::FadeOut:
            m_fade->update(deltaTime);
            if (m_fade->isFinished())
            {
                auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
                if (sceneManager)
                    sceneManager->changeScene(SceneType::InGame);
            }
            break;
        }
    }

    void Loading::draw()
    {
        // 暗いオーバーレイを描画
        m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(),
            core::utility::Color::argb(0x80, 0x00, 0x00, 0x00), true);

        if (m_fade)
            m_fade->draw(m_uiRenderer, m_screen);
    }

    void Loading::notifyLoadingComplete() noexcept
    {
        if (m_state == State::Loading)
            startFadeOut();
    }

    void Loading::startFadeOut() noexcept
    {
        m_fade = std::make_unique<ui::FadeTransition>(
            m_uiRenderer, m_screen, FADE_DURATION, false);
        m_state = State::FadeOut;
    }
}
