#pragma once
#include "IScene.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>

namespace game::scene
{
	class LockscreenView;

	/**
	 * @brief ロック画面シーンのクラス
	 */
	class Lockscreen : public IScene
	{
	public:
		/**
		 * @brief Lockscreenのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 */
		Lockscreen(core::iface::IInputProvider& inputProvider,
			core::iface::IUIRenderer& uiRenderer,
			core::iface::IScreen& screen);

		/**
		 * @brief Lockscreenのデストラクタ
		 */
		~Lockscreen() noexcept;

		/**
		 * @brief シーンの更新処理
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief シーンの描画処理
		 */
		void draw() override;

	private:
		enum class State
		{
			FadeIn,
			Idle,
			Sliding
		};

		core::iface::IInputProvider& m_inputProvider;
		core::iface::IUIRenderer&    m_uiRenderer;
		core::iface::IScreen&        m_screen;
		std::unique_ptr<LockscreenView> m_lockscreenView;

		State m_state{ State::FadeIn };
		float m_offsetY{};
		std::unique_ptr<ui::FadeTransition> m_fade;

		static constexpr float SLIDE_DURATION = 0.7f;
		static constexpr float FADE_DURATION  = 0.5f;
	};
}
