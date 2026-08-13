#include "BlockDebrisSystem.h"
#include "game/component/stage/BlockDebrisComponent.h"
#include "game/component/stage/DestructibleComponent.h"
#include "core/ecs/Entity.h"
#include <algorithm>

namespace
{
	// 破片は「硬いものが砕けた欠片」として動かす。
	// 勢いを保ったまま飛び、床では跳ねずに転がって止まる、という挙動を狙う。
	// 空気抵抗を強くしたり大きく弾ませたりすると、軽くて柔らかいもの（スポンジ）に見える

	/// @brief 破片にかかる重力（ユニット/秒^2）
	///
	/// プレイヤーの落下（980）より重くする。ふわりと落ちると軽い物に見えるため
	constexpr float GRAVITY{ 2200.0f };

	/// @brief 床で跳ね返るときに残る速度の割合
	///
	/// 硬い欠片は床で弾まず、ぶつかった勢いを失って転がる。
	/// ここを大きくするとゴムまりのように跳ね回って重さが消える
	constexpr float BOUNCE_RETENTION{ 0.16f };

	/// @brief 床との摩擦で水平速度に掛ける係数
	constexpr float FLOOR_FRICTION{ 0.55f };

	/// @brief 空気抵抗（1秒あたりに失う速度の割合）
	///
	/// 硬く重い欠片は空気でほとんど減速しない。ここを大きくすると
	/// 飛び出した直後に失速し、軽いものが舞っているように見える
	constexpr float AIR_DRAG{ 0.15f };

	/// @brief 跳ねるたびに回転が落ち着く割合
	constexpr float SPIN_DAMPING{ 0.35f };

	/// @brief 静止したと見なす速さ（ユニット/秒）
	///
	/// これを下回ったら床の上で完全に止める。細かく震え続けると
	/// 「まだ動ける柔らかいもの」に見えてしまう
	constexpr float REST_SPEED{ 22.0f };

	/// @brief 寿命のうち、消え始めるまでの割合
	///
	/// 床に落ちて転がりきってから消えるように、後半だけフェードさせる
	constexpr float FADE_START_RATIO{ 0.65f };
} // namespace

namespace game::system::stage
{
	BlockDebrisSystem::BlockDebrisSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	    , m_renderer{ renderer }
	{
	}

	void BlockDebrisSystem::update(float deltaTime)
	{
		const auto entities{ m_componentManager.getAllEntities<component::stage::BlockDebrisComponent>() };
		for (const auto entityId : entities)
		{
			auto& debris{ m_componentManager.get<component::stage::BlockDebrisComponent>(entityId) };
			debris.m_elapsed += deltaTime;

			const auto* destructible{ m_componentManager.tryGet<component::stage::DestructibleComponent>(entityId) };
			if (destructible == nullptr)
				continue;

			// 飛散し終わったブロックは以後どこからも参照されないため破棄する
			if (debris.m_elapsed >= debris.m_lifetime)
			{
				// フェードのために変えた見た目を戻す。ハンドルはこのブロック専用の
				// 複製だが、戻さずに捨てるとプールへ返ったとき透けたまま出る
				m_renderer.resetModelAppearance(destructible->m_fracturedHandle);
				m_componentManager.removeAll(entityId);
				m_entityManager.destroy(core::ecs::Entity(entityId));
				continue;
			}

			// 後半でフェードさせる。破片は1体のモデルのフレームなので、
			// モデルごと薄くすれば全破片が揃って消える
			const float fadeStart{ debris.m_lifetime * FADE_START_RATIO };
			const float fade{ std::clamp((debris.m_elapsed - fadeStart) /
				                             (debris.m_lifetime - fadeStart),
				0.0f, 1.0f) };
			m_renderer.applyDeathDissolve(destructible->m_fracturedHandle, 0.0f, 1.0f - fade);

			for (auto& fragment : debris.m_fragments)
			{
				fragment.m_position += fragment.m_velocity * deltaTime;
				fragment.m_velocity.y -= GRAVITY * deltaTime;
				fragment.m_velocity = fragment.m_velocity * (1.0f - std::min(0.9f, AIR_DRAG * deltaTime));
				fragment.m_rotation += fragment.m_angular * deltaTime;

				if (fragment.m_position.y < debris.m_floorY)
				{
					fragment.m_position.y = debris.m_floorY;
					fragment.m_velocity.y = -fragment.m_velocity.y * BOUNCE_RETENTION;
					fragment.m_velocity.x *= FLOOR_FRICTION;
					fragment.m_velocity.z *= FLOOR_FRICTION;
					fragment.m_angular = fragment.m_angular * SPIN_DAMPING;

					// 転がりきったら完全に止める。細かく震え続けると重さが出ない
					if (fragment.m_velocity.lengthSq() < REST_SPEED * REST_SPEED)
					{
						fragment.m_velocity = {};
						fragment.m_angular = {};
					}
				}

				m_renderer.setModelFrameTransform(destructible->m_fracturedHandle, fragment.m_frameIndex,
				    fragment.m_pivot, fragment.m_position, fragment.m_rotation,
				    debris.m_modelScale);
			}
		}
	}
} // namespace game::system::stage
