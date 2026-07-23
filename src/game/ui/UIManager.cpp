#include "game/ui/UIManager.h"

namespace game::ui
{
	void UIManager::addElement(std::unique_ptr<IUIElement> element)
	{
		m_elements.push_back(std::move(element));
	}

	void UIManager::update()
	{
		for (auto& element : m_elements)
		{
			element->update();
		}
	}

	void UIManager::draw(core::iface::IUIRenderer& uiRenderer) const
	{
		for (const auto& element : m_elements)
		{
			element->draw(uiRenderer);
		}
	}
} // namespace game::ui