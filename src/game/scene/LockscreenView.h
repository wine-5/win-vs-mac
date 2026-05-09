#pragma once
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include <string>
#include "core/utility/Color.h"

namespace game::scene
{
	/**
	* @brief ロック画面の描画クラス
	*/
	class LockscreenView
	{
	public:
		/**
		 * @brief LockscreenViewのコンストラクタ
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 * @param mainFontName 使用するフォント名
		 */
		LockscreenView(core::iface::IUIRenderer& uiRenderer,
			core::iface::IScreen& screen,
			std::string mainFontName);

		/**
		 * @brief ヒントタイマーを更新する
		 * @param deltaTime 経過時間（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief ロック画面を描画する（通常は0、スライド中は負の値）
		 * @param offsetY スライドアップ用のY座標オフセット（通常は0、スライド中は負の値）
		 */
		void draw(int offsetY) const;

	private:
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		std::string m_mainFontName;

		float m_hintTimer{};

		static constexpr float HINT_PULSE_SPEED  = 2.0f;  // sin波の速さ
		static constexpr float TIME_Y_RATIO      = 0.42f; // 時刻のY位置
		static constexpr float DATE_Y_RATIO      = 0.55f; // 日付のY位置
		static constexpr float HINT_Y_RATIO      = 0.65f; // ヒントのY位置
		static constexpr float HINT_ALPHA_RANGE  = 0.3f;  // sin波の振幅
		static constexpr float HINT_ALPHA_MIN    = 0.7f;  // アルファ値の最小比率
		static constexpr unsigned int BG_COLOR = core::utility::Color::rgb(10, 27, 62);
	};
}