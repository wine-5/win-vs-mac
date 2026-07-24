#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/ICamera.h"
#include "core/interface/IInputProvider.h"

namespace game
{
	class GameManager; // DEBUG: デバッグモード参照用の前方宣言（リリース時に削除）
} // namespace game

namespace game::system::camera
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
		 * @param gameManager デバッグモード状態の参照（DEBUG: リリース時に削除）
		 */
		CameraSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId targetEntityId,
		    core::iface::IInputProvider& inputProvider,
		    core::iface::ICamera& camera,
		    GameManager& gameManager);

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
		GameManager& m_gameManager; // DEBUG: デバッグモード状態の参照（リリース時に削除）
	};
} // namespace game::system::camera
