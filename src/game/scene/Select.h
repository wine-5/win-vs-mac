#pragma once
#include "IScene.h"
#include "SceneType.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/ISelectWindowManager.h"
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
		 * @brief ポーズ中はセレクト画面のWindowを引っ込める
		 *
		 * Windowは常時最前面のため、出したままだとポーズメニューが裏に隠れて見えない
		 * @param isPaused ポーズ中ならtrue
		 */
		void onPauseChanged(bool isPaused) override;

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
		 * @brief タイトルへ戻る通知（Windowからのコールバック用）
		 *
		 * セレクト画面ではEscのポーズメニューを開けないため、
		 * デスクトップのアイコン／タスクバーがタイトルへ戻る唯一の入口になる
		 */
		void notifyBackToTitle() noexcept;

	  private:
		enum class State
		{
			FadeIn,
			Idle,
			FadeOut
		};

		/**
		 * @brief 暗転を始め、明けたら指定シーンへ移る
		 * @param nextScene 暗転後に移るシーン
		 */
		void startFadeOut(SceneType nextScene);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::iface::IResourceManager& m_resourceManager;

		std::unique_ptr<core::iface::ISelectWindowManager> m_windowManager;
		std::unique_ptr<ui::FadeTransition> m_fade;

		State m_state{ State::FadeIn };

		// 暗転が明けたあとに移るシーン。出撃ならLoading、タイトルへ戻るならTitle
		SceneType m_nextScene{ SceneType::Loading };

		static constexpr float FADE_DURATION = 0.5f;
	};
} // namespace game::scene
