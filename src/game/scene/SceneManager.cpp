#include "SceneManager.h"
#include "SceneFactory.h"

namespace game::scene
{
	SceneManager::SceneManager()
		: m_sceneFactory(std::make_unique<SceneFactory>())
		, m_currentScene{}
	{
	}

	SceneManager::~SceneManager() = default;

	void SceneManager::update(float deltaTime)
	{
		if (m_currentScene)
			m_currentScene->update(deltaTime);
	}

	void SceneManager::draw()
	{
		if (m_currentScene)
			m_currentScene->draw();
	}

	void SceneManager::changeScene(SceneType sceneType)
	{
		// SceneFactoryでシーンを生成（所有権はFactoryが保持）
		m_currentScene = m_sceneFactory->createScene(sceneType);
		m_currentSceneType = sceneType;
	}
}