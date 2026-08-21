#pragma once
#include "IScene.h"
#include "SceneType.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/ISelectWindowManager.h"
#include "core/interface/IInputProvider.h"
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
		 * @param inputProvider 入力インターフェース（パッドの操作をWindowへ渡すのに使う）
		 */
		Select(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IResourceManager& resourceManager,
		    std::unique_ptr<core::iface::ISelectWindowManager> windowManager,
		    core::iface::IInputProvider& inputProvider);

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
		 * @brief パッドでマウスカーソルを動かし、×で押す
		 *
		 * セレクト画面は押せる場所が多く、独立したWindowも並ぶ。枠を送る形だと
		 * Windowをまたぐ移動が煩雑になるため、カーソルそのものを動かす。
		 * 実際のクリックを送るので、×を素早く2回押せばデスクトップのアイコンも開ける。
		 *
		 * updateではなくフレーム単位で呼ぶのは、updateが固定ステップで
		 * 1フレームに0回のこともあり、押した瞬間を取りこぼすため
		 * @param deltaTime フレーム間の時間差（秒）
		 */
		void updateInput(float deltaTime) override;

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
		core::iface::IInputProvider& m_inputProvider;

		// カーソルの移動量の端数。1フレームぶんの移動は1ピクセルに満たないことが多く、
		// 切り捨てるとゆっくり倒したときに1ミリも動かなくなる
		float m_pointerRemainderX{ 0.0f };
		float m_pointerRemainderY{ 0.0f };

		std::unique_ptr<core::iface::ISelectWindowManager> m_windowManager;
		std::unique_ptr<ui::FadeTransition> m_fade;

		State m_state{ State::FadeIn };

		// 暗転が明けたあとに移るシーン。出撃ならLoading、タイトルへ戻るならTitle
		SceneType m_nextScene{ SceneType::Loading };

		static constexpr float FADE_DURATION = 0.5f;
	};
} // namespace game::scene
