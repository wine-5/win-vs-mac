#include "PauseMenuController.h"
#include "core/input/KeyCode.h"

namespace game::ui::pause
{
	PauseMenuController::PauseMenuController(core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_inputProvider{ inputProvider }
	    , m_screen{ screen }
	    , m_view{ uiRenderer, screen }
	{
	}

	void PauseMenuController::open(bool allowBackToTitle)
	{
		m_items.clear();
		m_items.push_back(PauseMenuAction::Resume);
		if (allowBackToTitle)
			m_items.push_back(PauseMenuAction::BackToTitle);
		m_items.push_back(PauseMenuAction::Quit);

		m_selectedIndex = 0;
		// 開いた瞬間のクリックで誤決定しないよう、現在の押下状態を引き継ぐ
		m_prevMouseLeft = m_inputProvider.isMouseLeftPressed();
	}

	PauseMenuAction PauseMenuController::update()
	{
		if (m_items.empty())
			return PauseMenuAction::None;

		const int itemCount{ static_cast<int>(m_items.size()) };

		// キーボード：↑↓で選択を移動する（端で止める）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::Up) && m_selectedIndex > 0)
			m_selectedIndex--;
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::Down) && m_selectedIndex < itemCount - 1)
			m_selectedIndex++;

		// マウス：ホバーで選択を移動する
		int mouseX{}, mouseY{};
		m_inputProvider.getMousePosition(mouseX, mouseY);
		const int hoveredIndex{ m_view.getItemIndexAt(mouseX, mouseY, itemCount) };
		if (hoveredIndex >= 0)
			m_selectedIndex = hoveredIndex;

		// マウス左クリックのエッジ検出（押した瞬間のみ）
		const bool mouseLeft{ m_inputProvider.isMouseLeftPressed() };
		const bool mouseClicked{ mouseLeft && !m_prevMouseLeft };
		m_prevMouseLeft = mouseLeft;

		// 決定：Enter、またはホバー中の項目をクリック
		const bool decided{ m_inputProvider.isKeyPressed(core::input::KeyCode::Enter) ||
			                (mouseClicked && hoveredIndex >= 0) };
		if (decided)
			return m_items[m_selectedIndex];

		return PauseMenuAction::None;
	}

	void PauseMenuController::draw()
	{
		m_view.draw(m_items, m_selectedIndex);
	}
} // namespace game::ui::pause
