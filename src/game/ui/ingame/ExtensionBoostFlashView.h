#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include <chrono>
#include <string>

namespace game::ui::ingame
{
	/**
	 * @brief ギャンブルボックスの当たりを引いた瞬間を画面全体で知らせるView
	 *
	 * 右下HUDとインベントリの見た目（紫・粒が倍）は「あとから見て分かる」ための表示で、
	 * 掛かった瞬間に気付かせる力は無い。壊した瞬間、プレイヤーはブロックを見ていて
	 * HUDを見ていないためである。そこで画面全体を一度だけ光らせ、
	 * 中央へ何が起きたのかを短く出す。
	 *
	 * イベントは購読しない。倍率は ExtensionInventoryComponent が持つ状態なので、
	 * 前フレームの値と突き合わせれば「上がった瞬間」が分かる。
	 * インベントリの増減表示と同じ考え方で、演出のためだけに配線を増やさない。
	 *
	 * @note 毎フレーム draw が呼ばれることが前提。呼ばれないフレームがあると
	 *       その間の変化を取りこぼす
	 */
	class ExtensionBoostFlashView
	{
	  public:
		/**
		 * @brief ExtensionBoostFlashViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager 倍率の読み出しに使うComponentManagerの参照
		 */
		ExtensionBoostFlashView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 倍率が上がっていれば演出を始め、演出中なら描画する
		 * @param playerId プレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

	  private:
		/**
		 * @brief 演出の開始からの経過秒数を返す
		 * @return 経過秒数（一度も始まっていなければ十分大きい値）
		 */
		[[nodiscard]] float elapsedSeconds() const;

		/**
		 * @brief 画面全体を覆う閃光を描く
		 * @param elapsed 演出開始からの経過秒数
		 */
		void drawFlash(float elapsed);

		/**
		 * @brief 中央へ「何が起きたか」を描く
		 * @param elapsed 演出開始からの経過秒数
		 * @param multiplier 掛かった倍率
		 */
		void drawMessage(float elapsed, float multiplier);

		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;

		// 前フレームの倍率。これより上がった瞬間だけ演出を始める
		float m_previousMultiplier{ 1.0f };

		// 演出を始めた時刻と、そのとき掛かった倍率
		std::chrono::steady_clock::time_point m_startTime{};
		float m_multiplier{ 1.0f };
		bool m_isPlaying{ false };

		// DxLibの描画はShift_JISを期待するため、日本語は生成時に一度だけ変換して持つ
		std::string m_message{};
	};
} // namespace game::ui::ingame
