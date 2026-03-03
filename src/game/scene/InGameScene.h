#pragma once
#include "IScene.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/SystemManager.h"
#include "game/ObjectFactory.h"
#include "game/component/RenderComponent.h"
#include "infrastructure/Camera.h"
#include "infrastructure/Renderer.h"
#include "infrastructure/ResourceManager.h"
#include "infrastructure/InputManager.h"


namespace game::scene
{
	/**
	 * @brief インゲームのシーンクラス
	 */
	class InGameScene : public IScene
	{
	public:
		InGameScene();
		void update(float deltaTime) override;

	private:
		core::ecs::EntityManager    m_entityManager;
		core::ecs::ComponentManager m_componentManager;
		core::ecs::SystemManager    m_systemManager;

		infrastructure::Camera m_camera;
		infrastructure::Renderer m_renderer;
		infrastructure::ResourceManager m_resourceManager;
		infrastructure::InputManager m_inputManager;

		game::ObjectFactory         m_objectFactory;

		// カメラ設定
		static constexpr float CAMERA_OFFSET_X = 0.0f;
		static constexpr float CAMERA_OFFSET_Y = 200.0f;
		static constexpr float CAMERA_OFFSET_Z = -300.0f;
	};
}