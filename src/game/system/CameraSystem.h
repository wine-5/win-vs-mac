#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/ICamera.h"
#include "core/interface/IInputProvider.h"

namespace game::system
{
	/**
	 * @brief マウス入力から3人称カメラを制御するSystem
	 *
	 * 対象EntityのCameraComponentをマウス移動量で更新し、
	 * yaw/pitch/距離からカメラのワールド座標を計算してICameraへ渡す。
	 */
	class CameraSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief CameraSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param targetEntityId 追従対象（プレイヤー）のEntityID
		 * @param inputProvider 入力のインターフェース
		 * @param camera カメラ装置のインターフェース
		 */
		CameraSystem(core::ecs::ComponentManager& componentManager,
		             core::ecs::EntityId targetEntityId,
		             core::iface::IInputProvider& inputProvider,
		             core::iface::ICamera& camera);

		/**
		 * @brief マウス入力に応じてカメラを更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_targetEntityId{};
		core::iface::IInputProvider& m_inputProvider;
		core::iface::ICamera& m_camera;
	};
} // namespace game::system
