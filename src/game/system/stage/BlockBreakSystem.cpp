#include "BlockBreakSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/stage/DestructibleComponent.h"
#include "core/utility/Log.h"
#include <algorithm>
#include <cmath>

namespace game::system::stage
{
	BlockBreakSystem::BlockBreakSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	    , m_playerId{ playerId }
	{
	}

	void BlockBreakSystem::update([[maybe_unused]] float deltaTime)
	{
		// 見るのは「振り始め」ではなく「当たり判定が解決された瞬間」。
		// プレイヤーの攻撃には playerData.json の attackWindup ぶんの溜めがあり、
		// 振り始めで判定すると剣が振り下ろされる前にひびが入る
		const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(m_playerId) };
		if (attack == nullptr || !attack->m_justResolved)
			return;

		// 弾はブロックを削らない。近づいて剣を振る理由を残すため
		if (m_componentManager.has<component::combat::ProjectileComponent>(m_playerId))
			return;

		const auto* playerTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(m_playerId) };
		if (playerTransform == nullptr)
			return;

		const auto blocks{ m_componentManager.getAllEntities<component::stage::DestructibleComponent>() };
		for (const auto blockId : blocks)
		{
			const auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
			if (destructible.isBroken())
				continue;

			const auto* blockTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(blockId) };
			if (blockTransform == nullptr)
				continue;

			// 箱の表面まで届いていれば当たりとする。中心同士の距離で測ると
			// 大きなブロックほど届きにくくなり、見た目と食い違う
			float reach{ attack->m_attackRange };
			if (const auto* collider{ m_componentManager.tryGet<component::combat::ColliderComponent>(blockId) })
				reach += std::max({ collider->m_size.x, collider->m_size.z }) * 0.5f;

			const float dx{ playerTransform->m_position.x - blockTransform->m_position.x };
			const float dz{ playerTransform->m_position.z - blockTransform->m_position.z };
			if (dx * dx + dz * dz > reach * reach)
				continue;

			hitBlock(blockId);
		}
	}

	void BlockBreakSystem::hitBlock(core::ecs::EntityId blockId)
	{
		auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
		++destructible.m_hitCount;

		// ひびを1段階進める。打撃回数とひびの枚数は一致しないので、
		// 対応付けは DestructibleComponent::crackStage が受け持つ
		const int stage{ destructible.crackStage() };
		if (stage > 0 && stage < static_cast<int>(destructible.m_crackTextures.size()))
		{
			const int texture{ destructible.m_crackTextures[stage] };
			if (texture != -1)
			{
				m_renderer.setModelTexture(destructible.m_intactHandle, texture);
				// 破片側にも同じ絵を貼っておく。破壊した瞬間に絵が戻ると割れて見えない
				m_renderer.setModelTexture(destructible.m_fracturedHandle, texture);
			}
		}
	}
} // namespace game::system::stage
