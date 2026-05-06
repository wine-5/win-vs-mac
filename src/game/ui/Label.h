#pragma once
#include "IUIElement.h"
#include "core/constant/UI.h"
#include <string>

namespace game::ui
{
	/**
	 * @brief テキストを描画するUI要素
	 */
	class Label : public IUIElement
	{
	public:
		/**
		 * @brief Labelのコンストラクタ
		 * @param text 表示するテキスト
		 * @param x X座標
		 * @param y Y座標
		 * @param color 文字色（ARGB）
		 * @param fontSize フォントサイズ（省略時はDEFAULT_FONT_SIZE）
		 */
		Label(std::string text, int x, int y, unsigned int color,
			int fontSize = core::constant::ui::DEFAULT_FONT_SIZE);

		void update() override;
		void draw(core::iface::IUIRenderer& uiRenderer) const override;
		void setVisible(bool visible) override;

		/**
		 * @brief テキストを変更する
		 * @param text 新しいテキスト
		 */
		void setText(const std::string& text);

	private:
		std::string m_text;
		int m_x{};
		int m_y{};
		int m_fontSize{};
		unsigned int m_color{};
		bool m_visible{};

	};
}