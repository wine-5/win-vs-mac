#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "HudPanel.h"
#include <array>
#include <chrono>

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム左下のプレイヤーステータス（HPバー＋能力値）を描画するView
	 *
	 * デザインはWindows 11（Fluent）に準拠する。パネルは角丸8pxの半透明サーフェス、
	 * HPバーはWindows 11のプログレスバーと同じく角丸のピル形状・単色で描く。
	 *
	 * 能力値はセレクト画面と同じ8項目を同じアイコンで見せる。8つ並べると画面を圧迫するため、
	 * 平常時は4項目ずつをスライドで入れ替え、Tabを押している間だけ8項目すべてを開く。
	 * インゲームはマウスカーソルを隠すので、押せる見た目のUI（ページのドット等）は置かない。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class PlayerHUDView
	{
	  public:
		/// @brief 表示する能力値の項目数（セレクト画面と同じ8項目）
		static constexpr int STAT_COUNT{ 8 };

		/**
		 * @brief PlayerHUDViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager HealthComponent等の読み出しに使うComponentManagerの参照
		 * @param resourceManager 能力値アイコンの読み込みに使うリソース管理インターフェース
		 */
		PlayerHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    core::iface::IResourceManager& resourceManager);

		/**
		 * @brief コンポーネントから読めない能力値を設定する
		 *
		 * 移動速度と弾の性能はSystemが値を抱えていてコンポーネントに無いため、
		 * 組み立て側（InGame）から渡してもらう。Itemで動かせるようになった時点で
		 * コンポーネントへ移し、この経路は畳む
		 * @param moveSpeed 移動速度
		 * @param projectileSpeed Window弾の弾速
		 * @param projectileRange Window弾の飛距離
		 */
		void setDerivedStats(float moveSpeed, float projectileSpeed, float projectileRange);

		/**
		 * @brief プレイヤーステータスを描画する
		 * @param playerId ステータスの読み出し元となるプレイヤーのEntityID
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

		/**
		 * @brief 8項目の現在値を集める
		 *
		 * HP・攻撃・防御・射程・会心はコンポーネントの生値を読み、
		 * 残りは setDerivedStats() で受け取った値を使う
		 * @param playerId プレイヤーのEntityID
		 * @return STAT_ORDER の並びに対応した現在値
		 */
		[[nodiscard]] std::array<float, STAT_COUNT> collectStats(core::ecs::EntityId playerId) const;

		/**
		 * @brief ページ送りと開閉の進行を更新する
		 *
		 * 値が変わった項目があれば、その項目を含むページへ即座に送って少しの間留める。
		 * Item取得の瞬間に裏のページだと気づけないため
		 * @param stats 今フレームの8項目の値
		 * @param isExpanded Tabが押されているか
		 * @param deltaTime 前回の描画からの経過秒数
		 */
		void updatePaging(const std::array<float, STAT_COUNT>& stats, bool isExpanded, float deltaTime);

		/**
		 * @brief 能力値の並びを描画する
		 *
		 * 平常時はページ送り（スライド）、Tabで開いている間は2ページを縦に並べる。
		 * どちらの状態かでレイアウトが変わるので、その分岐をここに集める
		 * @param x 並びの左上X座標
		 * @param y 並びの左上Y座標
		 * @param cellWidth セル1つぶんの幅
		 * @param stats 8項目の値
		 */
		void drawStats(int x, int y, int cellWidth, const std::array<float, STAT_COUNT>& stats);

		/**
		 * @brief 能力値1項目を描画する
		 * @param x セルの左上X座標
		 * @param y セルの左上Y座標
		 * @param index STAT_ORDER 上の位置
		 * @param value 表示する値
		 * @param alpha 不透明度（0〜255）
		 */
		void drawStatCell(int x, int y, int index, float value, int alpha);

		/**
		 * @brief 能力値を4項目ぶん横に並べて描画する
		 * @param x 並びの左上X座標
		 * @param y 並びの左上Y座標
		 * @param cellWidth セル1つぶんの幅
		 * @param stats 8項目の値
		 * @param page 描画するページ（0または1）
		 * @param alpha 不透明度（0〜255）
		 */
		void drawStatPage(int x, int y, int cellWidth,
		    const std::array<float, STAT_COUNT>& stats, int page, int alpha);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		HudPanel m_panel;

		// 能力値アイコンの画像ハンドル（STAT_ORDER と同じ並び）。読み込みに失敗した項目は-1
		std::array<int, STAT_COUNT> m_iconHandles{};

		// コンポーネントに無い能力値。setDerivedStats() で受け取る
		float m_moveSpeed{ 0.0f };
		float m_projectileSpeed{ 0.0f };
		float m_projectileRange{ 0.0f };

		// ページ送りの状態
		int m_page{ 0 };                 // 今表示しているページ（0または1）
		float m_pageTimer{ 0.0f };       // 次の切り替えまでの残り秒数
		float m_slideProgress{ 1.0f };   // 切り替えアニメの進行（1.0で完了）
		int m_slideFromPage{ 0 };        // 切り替え前のページ（スライドアウト側）
		float m_expandProgress{ 0.0f };  // Tabで開く進行（0.0で閉、1.0で全項目表示）
		float m_changeHighlight{ 0.0f }; // 値が変わった項目を強調する残り秒数
		int m_changedIndex{ -1 };        // 直近で値が変わった項目（無ければ-1）
		std::array<float, STAT_COUNT> m_previousStats{};
		bool m_hasPreviousStats{ false };

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
