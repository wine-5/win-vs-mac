#include "SceneManager.h"

namespace game::scene
{
	void SceneManager::update(float deltaTime)
	{
		if (m_currentScene)
			m_currentScene->update(deltaTime);
	}

	void SceneManager::changeScene(SceneType sceneType, std::unique_ptr<IScene> scene)
	{
		m_currentSceneType = sceneType;
		m_currentScene = std::move(scene);
	}
}