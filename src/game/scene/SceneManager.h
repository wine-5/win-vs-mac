#pragma once
#include "IScene.h"
#include "SceneType.h"
#include <memory>

namespace game::scene
{
	/**
	 * @brief シーン全体を管理するクラス
	 */
	class SceneManager
	{
	public:
		/**
		 * @brief 現在のシーンを更新する
		 * @param deltaTime フレーム間の時間差
		 */

		void update(float deltaTime);
		/**
		 * @brief シーンを変更する
		 * @param sceneType 変更先のシーンの種類
		 */

		void changeScene(SceneType sceneType, std::unique_ptr<IScene> scene);

	private:
		std::unique_ptr<IScene> m_currentScene{};
		SceneType m_currentSceneType{};
	};
}