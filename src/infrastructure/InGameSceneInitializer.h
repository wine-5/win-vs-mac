#pragma once
#include "game/scene/InGameScene.h"
#include "infrastructure/Camera.h"
#include "infrastructure/Renderer.h"
#include "infrastructure/Animator.h"
#include "infrastructure/ResourceManager.h"
#include "infrastructure/InputManager.h"
#include <memory>

namespace infrastructure
{
	/**
	* @brief InGameSceneの初期化を担当するクラス
	* 具象クラスを生成しInGameSceneにインターフェース経由で渡す
	*/
	class InGameSceneInitializer
	{
	public:
		/**
		 * @brief InGameSceneInitializerのコンストラクタ
		 */
		InGameSceneInitializer();
		
		/**
		 * @brief 初期化されたInGameSceneを取得する
		 * @return InGameSceneの参照
		 */
		game::scene::InGameScene& getScene();

	private:
		Camera                         m_camera;
		Renderer                       m_renderer;
		Animator                       m_animator;
		ResourceManager                m_resourceManager;
		InputManager                   m_inputManager;
		std::unique_ptr<game::scene::InGameScene> m_scene;
	};
}
