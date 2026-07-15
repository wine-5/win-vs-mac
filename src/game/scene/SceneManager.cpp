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
		// 前フレームで予約された古いシーンを破棄する
		// （シーンのコールスタック上での破棄を避けるため1フレーム遅延）
		if (m_pendingReset.has_value() && m_pendingReset.value() != m_currentSceneType)
		{
			m_sceneFactory->resetScene(m_pendingReset.value());
			m_pendingReset.reset();
		}

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
		// 現在シーンが存在する場合のみ、翌フレームでの遅延リセットを予約する
		if (m_currentScene != nullptr)
			m_pendingReset = m_currentSceneType;

		// SceneFactory でシーンを生成（所有権は Factory が保持）
		m_currentScene = m_sceneFactory->createScene(sceneType);
		m_currentSceneType = sceneType;
	}
} // namespace game::scene