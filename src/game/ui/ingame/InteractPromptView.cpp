#include "InteractPromptView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "game/component/movement/TransformComponent.h"
#include <algorithm>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// 吹き出しを対象の何ユニット上へ出すか。頭の上に浮かせて本体を隠さない
	constexpr float WORLD_OFFSET_Y{ 150.0f };

	// 吹き出しの見た目（1080p基準）
	constexpr int PADDING_X{ 18 };
	constexpr int PADDING_Y{ 12 };
	constexpr int RADIUS{ 6 };
	constexpr int TAIL_WIDTH{ 18 }; // 下向きの三角（対象を指す尻尾）
	constexpr int TAIL_HEIGHT{ 12 };

	constexpr unsigned int FILL_COLOR{ core::utility::Color::HUD_PANEL_FILL };
	constexpr int FILL_ALPHA{ 224 };
	constexpr unsigned int BORDER_COLOR{ core::utility::Color::HUD_ACCENT };

	// キーの表記を囲むバッジ。押すキーだけを先に目へ入れる
	constexpr int KEY_PADDING_X{ 10 };
	constexpr int KEY_GAP{ 12 }; // バッジと説明文の間隔
	constexpr unsigned int KEY_FILL_COLOR{ core::utility::Color::HUD_ACCENT };

	constexpr int FONT_SIZE{ 20 };

	// 浮かび上がる演出。ぱっと出ると視界の端で見落とす
	constexpr float APPEAR_SPEED{ 8.0f }; // 1.0へ到達する速さ（毎秒）
	constexpr int APPEAR_RISE{ 14 };      // 浮き上がる距離（1080p基準）

	// 画面の外にいる対象には出さない（深度が範囲外＝カメラの後ろ）
	constexpr float DEPTH_MIN{ 0.0f };
	constexpr float DEPTH_MAX{ 1.0f };
} // namespace

namespace game::ui::ingame
{
	InteractPromptView::InteractPromptView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IRenderer& renderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IInputProvider& inputProvider)
	    : m_uiRenderer{ uiRenderer }
	    , m_renderer{ renderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_inputProvider{ inputProvider }
	    , m_padButtonIcon{ uiRenderer }
	{
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		m_keyText = "F2";
		// Windowsの操作名（名前の変更）ではなく、ゲーム上で何が起きるかを書く。
		// 初見はWindowsの操作を知っていても、それがゲームで何になるのかは分からない
		m_actionText = converter ? converter->utf8ToShiftJis("拡張子を付け替える") : "拡張子を付け替える";
	}

	int InteractPromptView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void InteractPromptView::draw(core::ecs::EntityId targetId)
	{
		const auto now{ std::chrono::steady_clock::now() };
		const float deltaTime{ m_hasLastFrameTime
			                       ? std::chrono::duration<float>(now - m_lastFrameTime).count()
			                       : 0.0f };
		m_lastFrameTime = now;
		m_hasLastFrameTime = true;

		// 対象が変わったら出現をやり直す。別の端末へ移ったことを動きで示す
		if (targetId != m_lastTargetId)
		{
			m_appearProgress = 0.0f;
			m_lastTargetId = targetId;
		}

		if (targetId == core::ecs::INVALID_ENTITY_ID)
			return;

		const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(targetId) };
		if (transform == nullptr)
			return;

		m_appearProgress = std::min(1.0f, m_appearProgress + deltaTime * APPEAR_SPEED);

		const core::Vector3 worldPos{ transform->m_position.x,
			transform->m_position.y + WORLD_OFFSET_Y,
			transform->m_position.z };
		const core::Vector3 screenPos{ m_renderer.worldToScreen(worldPos) };

		// 深度が範囲外＝カメラの後ろ。振り返った瞬間に画面の変な位置へ出るのを防ぐ
		if (screenPos.z < DEPTH_MIN || screenPos.z > DEPTH_MAX)
			return;

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int fontSize{ scaled(FONT_SIZE) };
		const int actionWidth{ m_uiRenderer.getTextWidth(m_actionText.c_str(), fontSize) };

		// パッドを触っているなら□の記号、そうでなければキーの名前を出す
		const bool isPad{ m_inputProvider.getLastInputDevice() == core::input::InputDevice::GamePad };
		const int keyWidth{ isPad ? m_padButtonIcon.measure(ui::PadButton::Square, fontSize)
			                      : m_uiRenderer.getTextWidth(m_keyText.c_str(), fontSize) };

		const int keyBadgeWidth{ keyWidth + scaled(KEY_PADDING_X) * 2 };
		const int contentWidth{ keyBadgeWidth + scaled(KEY_GAP) + actionWidth };
		const int boxWidth{ contentWidth + scaled(PADDING_X) * 2 };
		const int boxHeight{ fontSize + scaled(PADDING_Y) * 2 };

		// 出現中は少し下から浮かび上がらせる
		const int rise{ static_cast<int>(scaled(APPEAR_RISE) * (1.0f - m_appearProgress)) };
		const int left{ static_cast<int>(screenPos.x) - boxWidth / 2 };
		const int top{ static_cast<int>(screenPos.y) - boxHeight - scaled(TAIL_HEIGHT) + rise };
		const int alpha{ static_cast<int>(FILL_ALPHA * m_appearProgress) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		m_uiRenderer.drawRoundedBox(left, top, boxWidth, boxHeight, scaled(RADIUS),
		    FILL_COLOR, true, 1);
		m_uiRenderer.drawRoundedBox(left, top, boxWidth, boxHeight, scaled(RADIUS),
		    BORDER_COLOR, false, 1);

		// 下向きの尻尾。どの物に対する案内なのかを指し示す
		const int tailCenterX{ left + boxWidth / 2 };
		const int tailTop{ top + boxHeight };
		m_uiRenderer.drawTriangle(tailCenterX - scaled(TAIL_WIDTH) / 2, tailTop,
		    tailCenterX + scaled(TAIL_WIDTH) / 2, tailTop,
		    tailCenterX, tailTop + scaled(TAIL_HEIGHT), FILL_COLOR, true);

		// キーのバッジ。押すキーだけを先に目へ入れる
		const int keyLeft{ left + scaled(PADDING_X) };
		m_uiRenderer.drawRoundedBox(keyLeft, top + scaled(PADDING_Y) / 2, keyBadgeWidth,
		    boxHeight - scaled(PADDING_Y), scaled(RADIUS), KEY_FILL_COLOR, true, 1);

		if (isPad)
		{
			m_padButtonIcon.draw(ui::PadButton::Square,
			    keyLeft + scaled(KEY_PADDING_X), top + scaled(PADDING_Y), fontSize);
		}
		else
		{
			m_uiRenderer.drawText(keyLeft + scaled(KEY_PADDING_X), top + scaled(PADDING_Y),
			    m_keyText.c_str(), core::utility::Color::HUD_INK, fontSize);
		}
		m_uiRenderer.drawText(keyLeft + keyBadgeWidth + scaled(KEY_GAP), top + scaled(PADDING_Y),
		    m_actionText.c_str(), core::utility::Color::HUD_INK, fontSize);

		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetFont();
	}
} // namespace game::ui::ingame
