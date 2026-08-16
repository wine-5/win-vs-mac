#include "PauseMenuController.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/SeType.h"

namespace game::ui::pause
{
	PauseMenuController::PauseMenuController(core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_inputProvider{ inputProvider }
	    , m_screen{ screen }
	    , m_inputMapper{ inputProvider }
	    , m_view{ uiRenderer, screen }
	{
	}

	void PauseMenuController::open(bool allowBackToTitle)
	{
		// 並びは Ctrl+Alt+Del のセキュリティオプション画面に合わせる。
		m_items.clear();
		m_items.push_back(PauseMenuAction::Settings);
		if (allowBackToTitle)
			m_items.push_back(PauseMenuAction::BackToTitle);
		m_items.push_back(PauseMenuAction::Quit);
		m_items.push_back(PauseMenuAction::Resume);

		m_selectedIndex = 0;
		// 開いた瞬間のキー・クリックで誤決定しないよう、現在の押下状態を引き継ぐ
		m_inputMapper.reset();
		m_prevMouseLeft = m_inputProvider.isMouseLeftPressed();
	}

	PauseMenuAction PauseMenuController::update(float deltaTime)
	{
		if (m_items.empty())
			return PauseMenuAction::None;

		m_inputMapper.update(deltaTime);

		const int itemCount{ static_cast<int>(m_items.size()) };
		const int previousIndex{ m_selectedIndex };

		// キーボード：↑↓で選択を移動する（端で止める）
		if (m_inputMapper.isTriggered(UiAction::NavigateUp) && m_selectedIndex > 0)
			m_selectedIndex--;
		if (m_inputMapper.isTriggered(UiAction::NavigateDown) && m_selectedIndex < itemCount - 1)
			m_selectedIndex++;

		// マウス：ホバーで選択を移動する
		int mouseX{}, mouseY{};
		m_inputProvider.getMousePosition(mouseX, mouseY);
		const int hoveredIndex{ m_view.getItemIndexAt(mouseX, mouseY, itemCount) };
		if (hoveredIndex >= 0)
			m_selectedIndex = hoveredIndex;

		// 右下の電源ボタン。実物と同じく、ここからも電源を切れる
		m_isPowerHovered = m_view.isOnPowerButton(mouseX, mouseY);

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };

		// 選択が動いたときだけ鳴らす。キーとマウスホバーのどちらで動いても同じ音にして、
		// 「今どこを選んでいるか」を操作方法によらず同じ手応えで返す
		if (audio && m_selectedIndex != previousIndex)
			audio->playSe(core::constant::SeType::UiKeyPress);

		// マウス左クリックのエッジ検出（押した瞬間のみ）
		const bool mouseLeft{ m_inputProvider.isMouseLeftPressed() };
		const bool mouseClicked{ mouseLeft && !m_prevMouseLeft };
		m_prevMouseLeft = mouseLeft;

		// 電源ボタンのクリックは項目を経由せず、そのまま終了へ送る
		if (mouseClicked && m_isPowerHovered)
		{
			if (audio)
				audio->playSe(core::constant::SeType::UiClick);
			return PauseMenuAction::Quit;
		}

		// 決定：Enter、またはホバー中の項目をクリック
		const bool decided{ m_inputMapper.isTriggered(UiAction::Confirm) ||
			                (mouseClicked && hoveredIndex >= 0) };
		if (decided)
		{
			if (audio)
				audio->playSe(core::constant::SeType::UiClick);
			return m_items[m_selectedIndex];
		}

		return PauseMenuAction::None;
	}

	void PauseMenuController::draw()
	{
		m_view.draw(m_items, m_selectedIndex, m_isPowerHovered);
	}
} // namespace game::ui::pause
