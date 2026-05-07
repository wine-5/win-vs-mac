#include "Select.h"
#include "SelectView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/ServiceLocator.h"

namespace game::scene
{
	Select::Select(core::iface::IInputProvider &inputProvider,
		core::iface::IUIRenderer &uiRenderer,
		core::iface::IScreen &screen,
		core::iface::IFileProvider &fileProvider,
		data::FileEquipmentData &fileEquipmentData)
	{
		m_view = std::make_unique<SelectView>(inputProvider, uiRenderer, screen, fileProvider, fileEquipmentData);
	}

	void Select::update(float deltaTime)
	{
		m_view->update(deltaTime);

		if (!m_view->isReadyToChange())
			return;

		switch (m_view->getNextAction())
		{
		case SelectView::Action::GoToLoading:
		{
			auto *sceneManager{core::ServiceLocator::get<SceneManager>()};
			sceneManager->changeScene(SceneType::Loading);
			break;
		}
		default:
			break;
		}
	}

	void Select::draw()
	{
		m_view->draw();
	}
}
