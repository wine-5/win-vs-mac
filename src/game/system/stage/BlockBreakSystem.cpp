#include "BlockBreakSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/stage/DestructibleComponent.h"
#include "game/component/stage/BlockDebrisComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "core/utility/Log.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace
{
	/// @brief 破片の初速（ユニット/秒）
	constexpr float BURST_SPEED{ 320.0f };

	/// @brief 破片へ加える上向きの初速の範囲（ユニット/秒）
	constexpr float BURST_LIFT_MIN{ 140.0f };
	constexpr float BURST_LIFT_MAX{ 360.0f };

	/// @brief 破片の回転速度の上限（ラジアン/秒）
	constexpr float SPIN_SPEED{ 8.0f };

	/// @brief モデル素材の実寸。ブロックのスケールは 実寸 / これ で決まっている
	constexpr float BASE_SIZE{ 100.0f };

	/**
	 * @brief 範囲内の一様乱数を返す
	 * @param min 最小値
	 * @param max 最大値
	 * @return min〜maxの乱数
	 */
	float randomRange(float min, float max)
	{
		static std::mt19937 engine{ std::random_device{}() };
		std::uniform_real_distribution<float> distribution{ min, max };
		return distribution(engine);
	}
} // namespace

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

		if (destructible.isBroken())
			breakBlock(blockId);
	}

	void BlockBreakSystem::breakBlock(core::ecs::EntityId blockId)
	{
		const auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(blockId) };

		// 見た目を「割ってあるモデル」へ差し替える。以降の描画はこちらになる
		auto* render{ m_componentManager.tryGet<component::visual::RenderComponent>(blockId) };
		if (render == nullptr || destructible.m_fracturedHandle == -1)
			return;
		render->m_modelHandle = destructible.m_fracturedHandle;

		// 壊れた瞬間から通り抜けられるようにする。当たり判定を残すと
		// 破片が飛んでいるのに見えない壁が立ったままになる
		m_componentManager.remove<component::combat::ColliderComponent>(blockId);

		component::stage::BlockDebrisComponent debris{};
		debris.m_modelScale = transform.m_scale.y;
		debris.m_floorY = transform.m_position.y - transform.m_scale.y * BASE_SIZE * 0.5f;

		const int frameCount{ m_renderer.getModelFrameCount(destructible.m_fracturedHandle) };
		debris.m_fragments.reserve(static_cast<std::size_t>(frameCount));

		for (int i{ 0 }; i < frameCount; ++i)
		{
			component::stage::BlockDebrisFragment fragment{};
			fragment.m_frameIndex = i;
			fragment.m_pivot = m_renderer.getModelFrameCenter(destructible.m_fracturedHandle, i);

			// 破片の重心はモデルのローカル座標なので、ブロックを置いた場所へ移す
			fragment.m_position = transform.m_position + fragment.m_pivot * debris.m_modelScale;

			// ブロックの中心から外向きに弾く。中心にぴったり重なった破片は
			// 向きが決まらないので上へ逃がす
			core::Vector3 direction{ fragment.m_position - transform.m_position };
			if (direction.lengthSq() <= 0.0f)
				direction = { 0.0f, 1.0f, 0.0f };

			fragment.m_velocity = direction.normalized() * (BURST_SPEED * randomRange(0.6f, 1.3f));
			fragment.m_velocity.y += randomRange(BURST_LIFT_MIN, BURST_LIFT_MAX);
			fragment.m_angular = {
				randomRange(-SPIN_SPEED, SPIN_SPEED),
				randomRange(-SPIN_SPEED, SPIN_SPEED),
				randomRange(-SPIN_SPEED, SPIN_SPEED)
			};
			debris.m_fragments.push_back(fragment);
		}

		m_componentManager.add<component::stage::BlockDebrisComponent>(blockId, debris);
	}
} // namespace game::system::stage
