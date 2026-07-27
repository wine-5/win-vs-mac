#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system::movement
{
	/**
	 * @brief 入力を元に速度を計算するSystem
	 */
	class MoveSystem : public core::ecs::ISystem
	{
	public:
	  /**
	   * @brief MoveSystemのコンストラクタ
	   *
	   * 移動速度はPlayerStatsComponentから毎フレーム読む。Itemで速度が上がっても
	   * そのまま効くようにするため、Systemが値を抱え込まない
	   * @param componentManager ComponentManagerの参照
	   * @param entityId 対象EntityのID
	   * @param dashMultiplier ダッシュ時の速度倍率
	   */
	  MoveSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId, float dashMultiplier);

	  /**
	   * @brief 入力を元に速度を計算する
	   * @param deltaTime フレーム間の時間差
	   */
	  void update(float deltaTime) override;

	private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_entityId{};
		float m_dashMultiplier{ 1.0f };
	};
} // namespace game::system::movement