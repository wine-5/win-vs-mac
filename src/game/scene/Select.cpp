#include "Select.h"
#include "SelectView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/ServiceLocator.h"

namespace game::scene
{
	Select::Select(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		core::iface::IFileProvider& fileProvider,
		core::iface::IJobProvider& jobProvider,
		data::FileEquipmentData& fileEquipmentData)
		: m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
		, m_fade{ std::make_unique<ui::FadeTransition>(uiRenderer, screen, FADE_DURATION, true) }
	{
		m_view = std::make_unique<SelectView>(
			inputProvider, uiRenderer, screen, fileEquipmentData, jobProvider,
			[this]() { startFadeOut(); },
			[&fileProvider, &fileEquipmentData](int slotIndex)
			{
				const std::string path{ fileProvider.selectFile() };
				if (!path.empty())
					fileEquipmentData.setFilePath(slotIndex, path);
			});
	}

	void Select::update(float deltaTime)
	{
		if (m_fade)
			m_fade->update(deltaTime);

		switch (m_state)
		{
		case State::FadeIn:
			if (m_fade && m_fade->isFinished())
			{
				m_fade.reset();
				m_state = State::Idle;
			}
			break;

		case State::Idle:
			m_view->update();
			break;

		case State::FadeOut:
			if (m_fade && m_fade->isFinished())
			{
				auto *sceneManager{core::ServiceLocator::get<SceneManager>()};
				sceneManager->changeScene(SceneType::Loading);
			}
			break;
		}
	}

	void Select::draw()
	{
		m_view->draw();
		if (m_fade)
			m_fade->draw();
	}

	void Select::startFadeOut()
	{
		if (m_state != State::Idle)
			return;
		m_fade = std::make_unique<ui::FadeTransition>(m_uiRenderer, m_screen, FADE_DURATION, false);
		m_state = State::FadeOut;
	}
}
