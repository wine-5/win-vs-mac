#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/ICamera.h"
#include "core/interface/IInputProvider.h"

namespace game
{
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
		/**
		 * @brief 壁に遮られない位置までカメラを手前へ寄せる
		 *
		 * 注視点からカメラへの線分が配置物を貫いていたら、その手前で止める。
		 * これをしないと壁の中にカメラが入り、プレイヤーが見えなくなる。
		 * @param lookTarget 注視点（プレイヤーの頭あたり）
		 * @param desiredDistance 遮蔽が無いときに保ちたい距離
		 * @param direction 注視点からカメラへ向かう単位ベクトル
		 * @return 実際に保てる距離
		 */
		float clampDistanceByWalls(const core::Vector3& lookTarget,
		    float desiredDistance, const core::Vector3& direction) const;

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_targetEntityId{};
		core::iface::IInputProvider& m_inputProvider;
		core::iface::ICamera& m_camera;

		// 壁で寄せた距離。寄るのは即座、戻るのは緩やかにするため前フレームの値を持つ
		float m_currentDistance{ 0.0f };
	};
} // namespace game::system::camera
