#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "game/factory/ProjectileFactory.h"

namespace game::system
{
	/**
	 * @brief プレイヤーの遠距離攻撃（Window投擲）の発射を担うSystem
	 *
	 * 発射入力とクールダウンを見て、カメラ前方へ弾を発射する。
	 * 弾の生成自体は ProjectileFactory に委譲する（敵の遠距離でも同じFactoryを使う）。
	 */
	class RangedAttackSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief RangedAttackSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param playerId プレイヤーのEntityID
		 * @param projectileFactory 弾生成ファクトリの参照
		 */
		RangedAttackSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId,
		    factory::ProjectileFactory& projectileFactory);

		/**
		 * @brief 発射入力に応じて弾を発射する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId{};
		factory::ProjectileFactory& m_projectileFactory;
		float m_cooldownTimer{ 0.0f };
	};
} // namespace game::system
