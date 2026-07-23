#pragma once
#include "IScene.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/ISelectWindowManager.h"
#include "core/constant/JobType.h"
#include <memory>

namespace game::scene
{
	/**
	 * @brief 選択シーンのクラス
	 */
	class Select : public IScene
	{
	  public:
		/**
		 * @brief Selectのコンストラクタ
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 * @param resourceManager リソース管理インターフェース
		 * @param windowManager セレクトウィンドウ管理インターフェース
		 */
		Select(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IResourceManager& resourceManager,
		    std::unique_ptr<core::iface::ISelectWindowManager> windowManager);

		/**
		 * @brief Selectのデストラクタ
		 */
		~Select() noexcept;

		/**
		 * @brief シーンの更新処理
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief シーンの描画処理
		 */
		void draw() override;

		/**
		 * @brief ウィンドウマネージャーを設定し、ウィンドウを作成する
		 * @param windowManager セレクトウィンドウ管理インターフェース
		 */
		void setWindowManager(std::unique_ptr<core::iface::ISelectWindowManager> windowManager) noexcept;

		/**
		 * @brief ゲーム開始通知（Windowからのコールバック用）
		 */
		void notifyGameStart() noexcept;

		/**
		 * @brief 職業選択通知（Windowからのコールバック用）
		 * @param jobType 選択された職業タイプ
		 */
		void notifyJobSelected(core::constant::JobType jobType) noexcept;

	  private:
		enum class State
		{
			FadeIn,
			Idle,
			FadeOut
		};

		void startFadeOut();

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::iface::IResourceManager& m_resourceManager;

		std::unique_ptr<core::iface::ISelectWindowManager> m_windowManager;
		std::unique_ptr<ui::FadeTransition> m_fade;

		State m_state{ State::FadeIn };

		static constexpr float FADE_DURATION = 0.5f;
	};
} // namespace game::scene
