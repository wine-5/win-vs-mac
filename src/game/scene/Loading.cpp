#include "Loading.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/utility/Color.h"
#include "core/base/ServiceLocator.h"

namespace game::scene
{
	Loading::Loading(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    std::unique_ptr<core::iface::IWindow> loadingWindow,
	    core::iface::IResourcePreloader& preloader)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_loadingWindow{ std::move(loadingWindow) }
	    , m_preloader{ preloader }
	{
    }

    Loading::~Loading() noexcept = default;

    void Loading::update(float deltaTime)
    {
        switch (m_state)
        {
        case State::FadeIn:
            m_state = State::Loading;
            break;

        case State::Loading:
			// 演出と先読みの両方が終わるのを待つ。
			// 先読みはApplicationのメインループが進めているのでここでは監視するだけ
			if (m_isAnimationFinished && m_preloader.isComplete())
				startFadeOut();
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
			m_fade->draw();
	}

    void Loading::notifyLoadingComplete() noexcept
    {
		// 実際の遷移判定はupdate()に任せる（先読みが残っている場合があるため）
		m_isAnimationFinished = true;
	}

    void Loading::startFadeOut() noexcept
    {
        m_fade = std::make_unique<ui::FadeTransition>(
            m_uiRenderer, m_screen, FADE_DURATION, false);
        m_state = State::FadeOut;
    }
} // namespace game::scene
