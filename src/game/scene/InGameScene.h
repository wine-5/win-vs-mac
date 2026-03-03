#pragma once
#include "IScene.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/SystemManager.h"
#include "game/ObjectFactory.h"
#include "game/component/RenderComponent.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IInputProvider.h"


namespace game::scene
{
	/**
	 * @brief インゲームのシーンクラス
	 */
	class InGameScene : public IScene
	{
	public:
		InGameScene(core::iface::ICamera& camera,
			core::iface::IRenderer& renderer,
			core::iface::IResourceManager& resourceManager,
			core::iface::IInputProvider& inputProvider);

		void update(float deltaTime) override;

	private:
		core::ecs::EntityManager    m_entityManager;
		core::ecs::ComponentManager m_componentManager;
		core::ecs::SystemManager    m_systemManager;

		core::iface::ICamera& m_camera;
		core::iface::IRenderer& m_renderer;
		core::iface::IResourceManager& m_resourceManager;
		core::iface::IInputProvider& m_inputProvider;

		game::ObjectFactory         m_objectFactory;

		// カメラ設定
		static constexpr float CAMERA_OFFSET_X = 0.0f;
		static constexpr float CAMERA_OFFSET_Y = 200.0f;
		static constexpr float CAMERA_OFFSET_Z = -300.0f;
	};
}