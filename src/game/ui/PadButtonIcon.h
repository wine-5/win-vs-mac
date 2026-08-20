#pragma once
#include "core/interface/IUIRenderer.h"

namespace game::ui
{
	/**
	 * @brief 操作の案内に出すパッドのボタン
	 *
	 * 表記を PlayStation の記号で持つ。Xbox 配列のパッドでも押す位置は同じなので、
	 * 記号さえ合っていれば場所は伝わる
	 */
	enum class PadButton
	{
		Cross,    // ×
		Circle,   // 〇
		Square,   // □
		Triangle, // △
		L1,
		R1,
		Options,
		Share,
		DPad,      // 十字キー
		LeftStick, // 左スティック
	};

	/**
	 * @brief パッドのボタン記号を図形で描くクラス
	 *
	 * 文字のフォントに頼らず図形で描くのは、小さくしたときに 〇 と □ が
	 * 潰れて見分けられなくなるため。
	 *
	 * 「× とはどういう図形か」の取り決め（線の太さ・内側の余白・色・幅）を
	 * ここ1箇所に集める。各Viewが自分で drawLine を呼ぶと、画面ごとに
	 * 太さや色が少しずつ違うものが並んでしまう
	 */
	class PadButtonIcon
	{
	  public:
		/**
		 * @brief PadButtonIconのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 */
		explicit PadButtonIcon(core::iface::IUIRenderer& uiRenderer) noexcept;

		/**
		 * @brief 記号を描くのに要する幅を返す
		 *
		 * 案内は右端揃えで描くものが多く、先に幅が分からないと位置を決められない。
		 * 記号を文字と混ぜると getTextWidth だけでは測れないため、これを使う
		 * @param button 描くボタン
		 * @param size 記号の高さ（ピクセル）
		 * @return 描画に要する幅（ピクセル）
		 */
		[[nodiscard]] int measure(PadButton button, int size) const;

		/**
		 * @brief パッドのボタン記号を描く
		 * @param button 描くボタン
		 * @param x 左端のX座標
		 * @param y 上端のY座標
		 * @param size 記号の高さ（ピクセル）
		 * @return 実際に描いた幅（ピクセル）
		 */
		int draw(PadButton button, int x, int y, int size);

	  private:
		core::iface::IUIRenderer& m_uiRenderer;
	};
} // namespace game::ui
