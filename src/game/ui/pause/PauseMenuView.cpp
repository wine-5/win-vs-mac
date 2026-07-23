#include "PauseMenuView.h"
#include "PauseMenuController.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include <utility>

namespace
{
	constexpr const char* MAIN_FONT_NAME{ "x12y16pxMaruMonica" };

	// 背景オーバーレイの暗さ（0〜255）
	constexpr int OVERLAY_ALPHA{ 160 };

	// レイアウト（画面高さに対する比率で解像度に依存しないようにする）
	constexpr float TITLE_Y_RATIO{ 0.28f };     // タイトル「PAUSE」のY位置
	constexpr float ITEMS_TOP_Y_RATIO{ 0.45f }; // 項目リストの先頭Y位置
	constexpr float ITEM_HEIGHT_RATIO{ 0.09f }; // 項目1つの高さ（間隔含む）
	constexpr float ITEM_WIDTH_RATIO{ 0.42f };  // 項目の当たり判定幅（画面幅比）

	// 色
	constexpr unsigned int SELECTED_COLOR{ 0xFFFFD700 };   // 選択中（金色）
	constexpr unsigned int UNSELECTED_COLOR{ 0xFFC8C8C8 }; // 非選択（薄いグレー）
} // namespace

namespace game::ui::pause
{
	PauseMenuView::PauseMenuView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
	}

	void PauseMenuView::draw(const std::vector<PauseMenuAction>& items, int selectedIndex)
	{
		// 画面全体を半透明の黒で覆い、背後のシーンをうっすら見せる
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, OVERLAY_ALPHA);
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(), core::utility::Color::BLACK, true);
		m_uiRenderer.resetBlendMode();

		m_uiRenderer.setFont(MAIN_FONT_NAME);

		// タイトル「PAUSE」を中央上部に描く
		const char* title{ "PAUSE" };
		const int titleFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::FONT_SIZE_TITLE_RATIO) };
		const int titleWidth{ m_uiRenderer.getTextWidth(title, titleFontSize) };
		m_uiRenderer.drawText((m_screen.getWidth() - titleWidth) / 2,
		    static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO),
		    title, core::utility::Color::WHITE, titleFontSize);

		// 項目リストを中央に描く（選択中は金色＋左に▶マーク）
		const int itemFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::FONT_SIZE_LARGE_RATIO) };
		// selectedIndex と比較するため i は int のまま、符号違いの比較は cmp_less で安全に行う
		for (int i{ 0 }; std::cmp_less(i, items.size()); ++i)
		{
			const bool isSelected{ i == selectedIndex };
			const unsigned int color{ isSelected ? SELECTED_COLOR : UNSELECTED_COLOR };

			const std::string label{ getLabel(items[i]) };
			const int labelWidth{ m_uiRenderer.getTextWidth(label.c_str(), itemFontSize) };

			int rectX{}, rectY{}, rectWidth{}, rectHeight{};
			getItemRect(i, rectX, rectY, rectWidth, rectHeight);

			const int textX{ (m_screen.getWidth() - labelWidth) / 2 };
			const int textY{ rectY + (rectHeight - itemFontSize) / 2 };
			m_uiRenderer.drawText(textX, textY, label.c_str(), color, itemFontSize);

			// 選択マーカー（テキストの左に描く）
			if (isSelected)
				m_uiRenderer.drawText(textX - itemFontSize, textY, ">", color, itemFontSize);
		}

		m_uiRenderer.resetFont();
	}

	int PauseMenuView::getItemIndexAt(int x, int y, int itemCount) const
	{
		for (int i{ 0 }; i < itemCount; ++i)
		{
			int rectX{}, rectY{}, rectWidth{}, rectHeight{};
			getItemRect(i, rectX, rectY, rectWidth, rectHeight);

			if (x >= rectX && x < rectX + rectWidth &&
			    y >= rectY && y < rectY + rectHeight)
				return i;
		}
		return -1;
	}

	void PauseMenuView::getItemRect(int index, int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		outWidth = static_cast<int>(m_screen.getWidth() * ITEM_WIDTH_RATIO);
		outHeight = static_cast<int>(m_screen.getHeight() * ITEM_HEIGHT_RATIO);
		outX = (m_screen.getWidth() - outWidth) / 2;
		outY = static_cast<int>(m_screen.getHeight() * ITEMS_TOP_Y_RATIO) + index * outHeight;
	}

	std::string PauseMenuView::getLabel(PauseMenuAction action) const
	{
		std::string label{};
		switch (action)
		{
		case PauseMenuAction::Resume: label = "ゲームに戻る"; break;
		case PauseMenuAction::BackToTitle: label = "タイトルへ戻る"; break;
		case PauseMenuAction::Quit: label = "ゲームを終了"; break;
		default: label = ""; break;
		}

		// DxLibのマルチバイト描画に合わせてUTF-8からShift-JISへ変換する
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (converter)
			label = converter->utf8ToShiftJis(label);
		return label;
	}
} // namespace game::ui::pause
