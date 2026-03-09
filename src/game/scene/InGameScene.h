#pragma once
#include "IScene.h"

/* core層のインクルード */
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/SystemManager.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IAnimator.h"

/* game層のインクルード */
#include "game/ObjectFactory.h"
#include "game/component/RenderComponent.h"
#include "game/data/PlayerData.h"
#include "game/stage/Ground.h"

#include <memory>


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
			core::iface::IAnimator& animator,
			core::iface::IResourceManager& resourceManager,
			core::iface::IInputProvider& inputProvider);

		void update(float deltaTime) override;

	private:
		core::ecs::EntityManager    m_entityManager;
		core::ecs::ComponentManager m_componentManager;
		core::ecs::SystemManager    m_systemManager;

		core::iface::ICamera& m_camera;
		core::iface::IRenderer& m_renderer;
		core::iface::IAnimator& m_animator;
		core::iface::IResourceManager& m_resourceManager;
		core::iface::IInputProvider& m_inputProvider;

		game::ObjectFactory         m_objectFactory;
		game::data::PlayerData m_playerData;

		std::unique_ptr <stage::Ground> m_ground;

		// カメラ設定
		static constexpr float CAMERA_OFFSET_X = 0.0f;
		static constexpr float CAMERA_OFFSET_Y = 200.0f;
		static constexpr float CAMERA_OFFSET_Z = -300.0f;
	};
}