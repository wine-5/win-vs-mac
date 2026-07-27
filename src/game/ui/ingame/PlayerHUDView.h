#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "HudPanel.h"
#include <chrono>

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム左下のプレイヤーステータス（HPバー）を描画するView
	 *
	 * デザインはWindows 11（Fluent）に準拠する。パネルは角丸8pxの半透明サーフェス、
	 * HPバーはWindows 11のプログレスバーと同じく角丸のピル形状・単色で描く。
	 * 画像は使わず、すべて UIRenderer のプリミティブで組む。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class PlayerHUDView
	{
	  public:
		/**
		 * @brief PlayerHUDViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager HealthComponentの読み出しに使うComponentManagerの参照
		 */
		PlayerHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief プレイヤーステータスを描画する
		 * @param playerId HPの読み出し元となるプレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief HPバーを描画する
		 *
		 * Windows 11のプログレスバーに倣い、溝と塗りをどちらも角丸のピル形状で描く。
		 * 残量に応じて緑→黄→赤と色が変わる
		 * @param x バー左上のX座標
		 * @param y バー左上のY座標
		 * @param width バーの幅（満タン時の長さ）
		 * @param height バーの高さ
		 * @param ratio HPの残量比（0.0〜1.0）
		 */
		void drawHealthBar(int x, int y, int width, int height, float ratio);

		/**
		 * @brief 前回の描画からの経過時間を求める
		 *
		 * 描画経路からしか呼ばれずdeltaTimeを受け取らないため、経過時間は壁時計から求める。
		 * 時間で進む演出が複数あるので、1フレームに一度ここで取ってから配る
		 * @return 前回の描画からの経過秒数（初回は0）
		 */
		[[nodiscard]] float tickDeltaTime();

		/**
		 * @brief 被弾の検知と、遅れて追従する残像バーの更新を行う
		 * @param ratio 今フレームのHP残量比（0.0〜1.0）
		 * @param deltaTime 前回の描画からの経過秒数
		 */
		void updateDamageReaction(float ratio, float deltaTime);

		/**
		 * @brief 被弾フラッシュの進行度を返す
		 * @return 0.0（被弾直後）〜1.0（終了）。再生中でなければ1.0
		 */
		[[nodiscard]] float getDamageFlashProgress() const;

		/**
		 * @brief 残りHPが少ないときに、バーを赤く脈動させる
		 *
		 * 平常時は光らせない。光っていること自体が危険の合図になるようにする
		 * @param x バー左上のX座標
		 * @param y バー左上のY座標
		 * @param width 塗られている部分の幅
		 * @param height バーの高さ
		 * @param radius 角丸の半径
		 * @param ratio HPの残量比（0.0〜1.0）
		 */
		void drawLowHealthPulse(int x, int y, int width, int height, int radius, float ratio);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		HudPanel m_panel;

		// 被弾演出の状態。実HPより遅れて縮む残像バーで「今どれだけ削られたか」を見せる
		float m_displayedRatio{ -1.0f }; // 負の値は未初期化（初回の描画で実HPに合わせる）
		float m_lastRatio{ -1.0f };      // 前フレームの実HP。被弾の瞬間の検知に使う
		std::chrono::steady_clock::time_point m_lastDamageTime{};
		// 低HPの脈動の基準時刻（生成時から連続して進める）
		std::chrono::steady_clock::time_point m_startTime{ std::chrono::steady_clock::now() };
		std::chrono::steady_clock::time_point m_lastFrameTime{};
		bool m_hasLastFrameTime{ false };
		bool m_isDamageFlashing{ false };
	};
} // namespace game::ui::ingame
