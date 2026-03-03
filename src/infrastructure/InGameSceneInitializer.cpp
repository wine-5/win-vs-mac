#include "InGameSceneInitializer.h"

namespace infrastructure
{
	InGameSceneInitializer::InGameSceneInitializer()
	{
		m_scene = std::make_unique<game::scene::InGameScene>(
			m_camera,
			m_renderer,
			m_animator,
			m_resourceManager,
			m_inputManager
		);
	}

	game::scene::InGameScene& InGameSceneInitializer::getScene()
	{
		return *m_scene;
	}
}