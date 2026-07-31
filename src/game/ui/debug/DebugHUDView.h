#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/constant/DebugFlags.h"
#include <chrono>

namespace core::iface
{
	class IPerformanceDataProvider; // 前方宣言
	class IEffectFactory;           // DEBUG: 前方宣言（リリース時に削除）
	class IRenderer;                // 前方宣言
} // namespace core::iface

namespace game
{
	class GameManager;  // 前方宣言
	class PauseManager; // 前方宣言
} // namespace game

namespace game::ui::debug
{
	/**
	 * @brief DEBUG: 常時表示のデバッグHUD（右上の統計・左上のカメラ状態ラベル）を担当するView
	 *
	 * FPS・フレーム時間・Entity数・CPU/メモリ使用率など、負荷の原因特定に使う情報を
	 * 右上にまとめて表示する。またデバッグカメラ（F1）・シーンビュー（F2）中は
	 * 左上に操作方法のラベルを表示する。
	 * リリース時はこのクラスごと削除する。
	 */
	class DebugHUDView
	{
	  public:
		/**
		 * @brief DebugHUDViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager Entity数の集計に使うComponentManagerの参照
		 * @param gameManager デバッグモード状態の参照
		 * @param pauseManager シーンビュー状態の参照
		 * @param perfProvider CPU/メモリ使用率の取得元
		 * @param effectFactory 同時再生中のエフェクト数の取得元（DEBUG: リリース時に削除）
		 * @param renderer 描画コール数の取得元
		 */
		DebugHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    GameManager& gameManager,
		    PauseManager& pauseManager,
		    core::iface::IPerformanceDataProvider& perfProvider,
		    core::iface::IEffectFactory& effectFactory,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief ゲーム更新（固定タイムステップ）が1回行われたことを記録する
		 * @details 更新レート（UPS）の算出に使う。Applicationの累積器により更新回数は
		 * 描画フレーム数と一致しないため、描画側とは別に数える必要がある。
		 * シーンのupdateから毎回呼ぶこと
		 */
		void countGameUpdate();

		/**
		 * @brief FPS計測とパフォーマンスデータの定期更新を行う（描画フレームごとに呼ぶ）
		 * @details 必ず描画側（drawの経路）から呼ぶこと。シーンのupdateから呼ぶと
		 * Applicationの固定タイムステップにより呼び出し回数が毎秒60回に固定されてしまい、
		 * 実際の描画が何FPS出ていても常に60と表示されてしまう。
		 * また計測には引数のdeltaTimeではなく壁時計時間（std::chrono）を直接使う
		 */
		void updateOnRenderFrame();

		/**
		 * @brief HUDを描画する
		 *
		 * 非表示のときは何も描かない（FPS等の計測自体は続ける）
		 * @param enemyCount 現在の敵の数（EnemyFactoryが管理する一覧のサイズ）
		 */
		void draw(int enemyCount);

		/**
		 * @brief 表示するかどうかを設定する
		 * @param visible 表示するならtrue
		 */
		void setVisible(bool visible) noexcept;

		/**
		 * @brief 表示中かどうかを返す
		 * @return 表示中ならtrue
		 */
		[[nodiscard]] bool isVisible() const noexcept;

	  private:
		/**
		 * @brief FPS・Entity数・CPU/メモリ使用率を右上に表示する
		 * @param enemyCount 現在の敵の数
		 */
		void drawStats(int enemyCount);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		GameManager& m_gameManager;
		PauseManager& m_pauseManager;
		core::iface::IPerformanceDataProvider& m_perfProvider;
		core::iface::IEffectFactory& m_effectFactory; // DEBUG: リリース時に削除
		core::iface::IRenderer& m_renderer;

		// FPS計測用（直近区間のフレーム数を数えて一定間隔ごとに算出する。瞬間値だと表示が揺れるため）
		// 実際の壁時計時間を使うため、Applicationの固定タイムステップに関わらず正確な値になる
		std::chrono::steady_clock::time_point m_lastUpdateTime{};
		bool m_hasLastUpdateTime{ false };
		int m_fpsFrameAccum{ 0 };
		int m_updateAccum{ 0 }; // 区間中のゲーム更新回数（UPS算出用。描画フレーム数とは一致しない）
		float m_fpsTimeAccum{ 0.0f };
		float m_displayFps{ 0.0f };
		float m_displayFrameMs{ 0.0f };
		float m_displayUps{ 0.0f };

		// パフォーマンスデータ（CPU/メモリ）の更新間隔管理
		float m_perfUpdateTimer{ 0.0f };

		// 表示するかどうか。既定値の切り替えは DebugFlags.h で行う
		bool m_isVisible{ core::constant::SHOW_DEBUG_HUD };

		static constexpr float FPS_UPDATE_INTERVAL{ 0.5f };  // FPS表示の更新間隔（秒）
		static constexpr float PERF_UPDATE_INTERVAL{ 1.0f }; // CPU/メモリ取得の更新間隔（秒）
	};
} // namespace game::ui::debug
