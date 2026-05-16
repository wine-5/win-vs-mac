#include "Select.h"
#include "SelectView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "game/GameManager.h"

namespace game::scene
{
	Select::Select(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		core::iface::IFileProvider& fileProvider,
		core::iface::IResourceManager& resourceManager,
		data::FileEquipmentData& fileEquipmentData)
		: m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
		, m_resourceManager{ resourceManager }
		, m_fade{ std::make_unique<ui::FadeTransition>(uiRenderer, screen, FADE_DURATION, true) }
	{
		auto& jobSelectionData{ game::GameManager::getInstance().getJobSelectionData() };

		m_view = std::make_unique<SelectView>(
			inputProvider, uiRenderer, screen, fileEquipmentData,
			resourceManager, jobSelectionData,
			[this]() { startFadeOut(); },
			[&fileProvider, &fileEquipmentData](int slotIndex)
			{
				const std::string path{ fileProvider.selectFile() };
				if (!path.empty())
					fileEquipmentData.setFilePath(slotIndex, path);
			},
			[this](int jobIdInt)
			{
				const core::constant::JobType jobType{ static_cast<core::constant::JobType>(jobIdInt) };
				auto& jobSelectionData{ game::GameManager::getInstance().getJobSelectionData() };
				jobSelectionData.setSelectedJobType(jobType);
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
				auto* sceneManager{ core::base::ServiceLocator::get<SceneManager>() };
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

