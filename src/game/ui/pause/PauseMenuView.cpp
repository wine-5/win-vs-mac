#include "PauseMenuView.h"
#include "PauseMenuController.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include <algorithm>
#include <utility>

namespace
{
	using Color = core::utility::Color;

	// 背景の暗さ。実物は一色で塗り潰す。ゲームでは背後の輪郭がかすかに残る程度に留める。
	// これより薄いと裏のボタンや文字が読めてしまい、どちらを操作する画面なのか分からなくなる
	constexpr int OVERLAY_ALPHA{ 240 };

	// レイアウト（画面サイズに対する比率で解像度に依存しないようにする）。
	// 実物と同じく、項目のまとまりを画面の中央へ置き、文字はその中で左揃えにする
	constexpr float ITEM_WIDTH_RATIO{ 0.19f };         // 項目の幅（画面幅比）
	constexpr float ITEM_HEIGHT_RATIO{ 0.056f };       // 項目1つの高さ（間隔含む）
	constexpr float ITEMS_TOP_Y_RATIO{ 0.32f };        // 項目リストの先頭Y位置
	constexpr float ITEM_TEXT_PADDING_RATIO{ 0.008f }; // 枠の内側の左余白

	// 最後の「キャンセル」だけ、上の項目群から離して置く（実物と同じ間の空け方）
	constexpr float CANCEL_GAP_RATIO{ 0.045f };

	// 選択中を示す白い枠線の太さ（画面高さ比）
	constexpr float FOCUS_RING_RATIO{ 0.0022f };

	// 右下の電源アイコン
	constexpr float POWER_CENTER_X_RATIO{ 0.955f };
	constexpr float POWER_CENTER_Y_RATIO{ 0.945f };
	constexpr float POWER_RADIUS_RATIO{ 0.018f };

	// 文字サイズ（画面高さ比）
	constexpr float FONT_ITEM_RATIO{ 0.028f };
} // namespace

namespace game::ui::pause
{
	PauseMenuView::PauseMenuView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
	}

	void PauseMenuView::draw(const std::vector<PauseMenuAction>& items, int selectedIndex, bool isPowerHovered)
	{
		const int screenWidth{ m_screen.getWidth() };
		const int screenHeight{ m_screen.getHeight() };

		// Ctrl+Alt+Del のセキュリティオプション画面に見立てる。
		// 真っ黒にすると単なる暗転に見えるので、青を残して「OSの画面へ切り替わった」ように見せる
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, OVERLAY_ALPHA);
		m_uiRenderer.drawBox(0, 0, screenWidth, screenHeight, Color::PAUSE_BACKGROUND_NAVY, true);
		m_uiRenderer.resetBlendMode();

		// 設定画面と同じ書体で描く。ここから開く設定がOSのウィンドウなので、
		// 手前のこの画面だけゲーム書体だと、同じ流れの中で画風が切り替わってしまう
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);

		drawItems(items, selectedIndex);
		drawPowerButton(isPowerHovered);

		m_uiRenderer.resetFont();
	}

	void PauseMenuView::drawItems(const std::vector<PauseMenuAction>& items, int selectedIndex) const
	{
		const int itemCount{ static_cast<int>(items.size()) };
		const int fontSize{ static_cast<int>(m_screen.getHeight() * FONT_ITEM_RATIO) };
		const int padding{ static_cast<int>(m_screen.getWidth() * ITEM_TEXT_PADDING_RATIO) };
		const int ringThickness{ std::max(1, static_cast<int>(m_screen.getHeight() * FOCUS_RING_RATIO)) };

		// selectedIndex と比較するため i は int のまま、符号違いの比較は cmp_less で安全に行う
		for (int i{ 0 }; std::cmp_less(i, items.size()); ++i)
		{
			int rectX{}, rectY{}, rectWidth{}, rectHeight{};
			getItemRect(i, itemCount, rectX, rectY, rectWidth, rectHeight);

			// 最後の「キャンセル」だけ、実物と同じく面のあるボタンにする。
			// 上の項目が「これから何かをする」のに対し、これは「やめる」なので見た目を分ける
			const bool isCancel{ isCancelIndex(i, itemCount) };
			if (isCancel)
			{
				m_uiRenderer.drawRoundedBox(rectX, rectY, rectWidth, rectHeight,
				    std::max(2, ringThickness * 2), Color::PAUSE_CANCEL_BUTTON, true, 1);
			}

			// 選択中は白い枠で囲む。実物のフォーカス表示がこの形
			if (i == selectedIndex)
			{
				m_uiRenderer.drawBox(rectX, rectY, rectWidth, ringThickness, Color::WHITE, true);
				m_uiRenderer.drawBox(rectX, rectY + rectHeight - ringThickness, rectWidth, ringThickness, Color::WHITE, true);
				m_uiRenderer.drawBox(rectX, rectY, ringThickness, rectHeight, Color::WHITE, true);
				m_uiRenderer.drawBox(rectX + rectWidth - ringThickness, rectY, ringThickness, rectHeight, Color::WHITE, true);
			}

			// キャンセルだけ中央揃え。上の項目は左揃え（実物と同じ）
			const std::string label{ getLabel(items[i]) };
			const int textX{ isCancel
				                 ? rectX + (rectWidth - m_uiRenderer.getTextWidth(label.c_str(), fontSize)) / 2
				                 : rectX + padding };

			m_uiRenderer.drawText(textX, rectY + (rectHeight - fontSize) / 2,
			    label.c_str(), Color::WHITE, fontSize);
		}
	}

	void PauseMenuView::getPowerCircle(int& outCenterX, int& outCenterY, int& outRadius) const
	{
		outCenterX = static_cast<int>(m_screen.getWidth() * POWER_CENTER_X_RATIO);
		outCenterY = static_cast<int>(m_screen.getHeight() * POWER_CENTER_Y_RATIO);
		outRadius = static_cast<int>(m_screen.getHeight() * POWER_RADIUS_RATIO);
	}

	bool PauseMenuView::isOnPowerButton(int x, int y) const
	{
		int centerX{}, centerY{}, radius{};
		getPowerCircle(centerX, centerY, radius);

		// 円ぴったりだと狙いにくいので、少し広めの正方形で拾う
		const int half{ radius + radius / 2 };
		return x >= centerX - half && x < centerX + half &&
		       y >= centerY - half && y < centerY + half;
	}

	void PauseMenuView::drawPowerButton(bool isHovered) const
	{
		int centerX{}, centerY{}, radius{};
		getPowerCircle(centerX, centerY, radius);

		const int thickness{ std::max(2, radius / 7) };
		const unsigned int color{ isHovered ? Color::WHITE : Color::SETTINGS_TEXT_SECONDARY };

		// 乗っているときは丸い下地を敷く。押せる場所だと分かるようにする
		if (isHovered)
		{
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, 30);
			m_uiRenderer.drawCircle(centerX, centerY, radius + radius / 2, Color::WHITE, true, 1);
			m_uiRenderer.resetBlendMode();
		}

		m_uiRenderer.drawCircle(centerX, centerY, radius, color, false, thickness);

		// 円の上を背景色で欠けさせ、そこへ縦棒を立てるのが電源の記号
		m_uiRenderer.drawBox(centerX - thickness, centerY - radius - thickness,
		    thickness * 2, thickness * 2, Color::PAUSE_BACKGROUND_NAVY, true);
		m_uiRenderer.drawBox(centerX - thickness / 2, centerY - radius - thickness / 2,
		    std::max(1, thickness), radius, color, true);
	}

	int PauseMenuView::getItemIndexAt(int x, int y, int itemCount) const
	{
		for (int i{ 0 }; i < itemCount; ++i)
		{
			int rectX{}, rectY{}, rectWidth{}, rectHeight{};
			getItemRect(i, itemCount, rectX, rectY, rectWidth, rectHeight);

			if (x >= rectX && x < rectX + rectWidth &&
			    y >= rectY && y < rectY + rectHeight)
				return i;
		}
		return -1;
	}

	bool PauseMenuView::isCancelIndex(int index, int itemCount) const noexcept
	{
		// 「キャンセル」は必ず最後に置く（PauseMenuController::open が並べる順）
		return index == itemCount - 1;
	}

	void PauseMenuView::getItemRect(int index, int itemCount,
	    int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		outWidth = static_cast<int>(m_screen.getWidth() * ITEM_WIDTH_RATIO);
		outHeight = static_cast<int>(m_screen.getHeight() * ITEM_HEIGHT_RATIO);
		outX = (m_screen.getWidth() - outWidth) / 2;
		outY = static_cast<int>(m_screen.getHeight() * ITEMS_TOP_Y_RATIO) + index * outHeight;

		// 最後のキャンセルだけ、上の項目群から離す
		if (isCancelIndex(index, itemCount))
			outY += static_cast<int>(m_screen.getHeight() * CANCEL_GAP_RATIO);
	}

	std::string PauseMenuView::getLabel(PauseMenuAction action) const
	{
		// Windows のセキュリティオプション画面と同じ語で見せる。
		// 「ゲームに戻る」ではなく「キャンセル」なのは、あの画面がそう書いてあるため
		switch (action)
		{
		case PauseMenuAction::Resume: return getDrawableText("キャンセル");
		case PauseMenuAction::Settings: return getDrawableText("設定");
		case PauseMenuAction::BackToTitle: return getDrawableText("サインアウト");
		case PauseMenuAction::Quit: return getDrawableText("シャットダウン");
		default: return std::string{};
		}
	}

	std::string PauseMenuView::getDrawableText(const char* utf8) const
	{
		// DxLibのマルチバイト描画に合わせてUTF-8からShift-JISへ変換する
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (!converter)
			return std::string{ utf8 };

		return converter->utf8ToShiftJis(utf8);
	}
} // namespace game::ui::pause
