#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace core::iface
{
	class IPerformanceDataProvider; // 前方宣言
}

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
		 */
		DebugHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    GameManager& gameManager,
		    PauseManager& pauseManager,
		    core::iface::IPerformanceDataProvider& perfProvider);

		/**
		 * @brief FPS計測とパフォーマンスデータの定期更新を行う
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime);

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

		// FPS計測用（直近区間のフレーム数を数えて一定間隔ごとに算出する。瞬間値だと表示が揺れるため）
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
