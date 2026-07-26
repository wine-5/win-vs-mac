#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

namespace game::system::movement
{
	/**
	 * @brief プレイヤーの足音を一歩ごとに鳴らすSystem
	 *
	 * 時間ではなく「進んだ距離」で歩数を数える。歩幅を一定にすることで、
	 * 歩きとダッシュで音の間隔が自動的に変わり、速さと音が食い違わなくなる。
	 * 位置が確定してから数えたいので、移動・接地の解決より後に更新すること。
	 */
	class FootstepSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param playerId 足音を鳴らす対象（プレイヤー）のEntityID
		 */
		FootstepSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId playerId);

		/**
		 * @brief 進んだ距離を数え、歩幅ぶん進むごとに足音を鳴らす
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;

		core::Vector3 m_lastPosition{};    // 前フレームの位置（移動量を出すために持つ）
		bool m_hasLastPosition{ false };   // 初回フレームは移動量を出せないため区別する
		float m_distanceSinceStep{ 0.0f }; // 最後に足音を鳴らしてから進んだ水平距離
	};
} // namespace game::system::movement
