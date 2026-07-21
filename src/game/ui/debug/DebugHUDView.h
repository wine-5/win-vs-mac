#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <chrono>

namespace core::iface
{
	class IPerformanceDataProvider; // 前方宣言
	class IEffectFactory;           // DEBUG: 前方宣言（リリース時に削除）
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
		 */
		DebugHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    GameManager& gameManager,
		    PauseManager& pauseManager,
		    core::iface::IPerformanceDataProvider& perfProvider,
		    core::iface::IEffectFactory& effectFactory);

		/**
		 * @brief FPS計測とパフォーマンスデータの定期更新を行う
		 * @details FPSは引数のdeltaTimeではなく内部で壁時計時間（std::chrono）を直接計測する。
		 * Applicationのゲームループは固定タイムステップ（常に1/60として進める）のため、
		 * 引数のdeltaTimeは実際の処理時間を表しておらず、それを使うと重い処理をしていても
		 * 常に60FPSと表示されてしまう。
		 */
		void update();

		/**
		 * @brief HUDを描画する
		 * @param enemyCount 現在の敵の数（EnemyFactoryが管理する一覧のサイズ）
		 */
		void draw(int enemyCount);

	  private:
		/**
		 * @brief デバッグカメラ（F1）・シーンビュー（F2）の状態と操作方法を左上に表示する
		 */
		void drawCameraLabel();

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

		// FPS計測用（直近区間のフレーム数を数えて一定間隔ごとに算出する。瞬間値だと表示が揺れるため）
		// 実際の壁時計時間を使うため、Applicationの固定タイムステップに関わらず正確な値になる
		std::chrono::steady_clock::time_point m_lastUpdateTime{};
		bool m_hasLastUpdateTime{ false };
		int m_fpsFrameAccum{ 0 };
		float m_fpsTimeAccum{ 0.0f };
		float m_displayFps{ 0.0f };
		float m_displayFrameMs{ 0.0f };

		// パフォーマンスデータ（CPU/メモリ）の更新間隔管理
		float m_perfUpdateTimer{ 0.0f };

		static constexpr float FPS_UPDATE_INTERVAL{ 0.5f };  // FPS表示の更新間隔（秒）
		static constexpr float PERF_UPDATE_INTERVAL{ 1.0f }; // CPU/メモリ取得の更新間隔（秒）
	};
} // namespace game::ui::debug
