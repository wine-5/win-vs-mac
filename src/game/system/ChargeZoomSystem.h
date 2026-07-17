#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
	/**
	 * @brief 溜め攻撃中にFOVを絞ってプレイヤーへ寄っていく演出を担うSystem
	 *
	 * PlayerChargeComponentの溜め率を読み、CameraEffectComponentの
	 * m_fovScale（FOV倍率）だけを書き込むdriver System。
	 * 実際のカメラ合成はCameraSystemが行う。
	 */
	class ChargeZoomSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief ChargeZoomSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param playerId プレイヤーのEntityID
		 */
		ChargeZoomSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 溜め率に応じてFOV倍率を目標値へ滑らかに補間する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;
	};
} // namespace game::system
