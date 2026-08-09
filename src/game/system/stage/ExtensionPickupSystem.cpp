#include "ExtensionPickupSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/stage/ExtensionPickupComponent.h"
#include "game/event/InGameEvents.h"
#include <algorithm>
#include <cmath>

namespace
{
	/// @brief 欠片にかかる重力（ユニット/秒^2）
	constexpr float GRAVITY{ 1600.0f };

	/// @brief 着地したと見なす落下速度の下限（ユニット/秒）
	///
	/// これより遅く落ちてきたら、跳ねずにその場で浮遊へ移る
	constexpr float LANDING_SPEED{ 40.0f };

	/// @brief 着地したときに残る跳ね返りの割合
	constexpr float BOUNCE_RETENTION{ 0.35f };

	/// @brief 着地後の浮遊（上下）の速さと幅
	constexpr float BOB_SPEED{ 2.6f };
	constexpr float BOB_AMPLITUDE{ 9.0f };

	/// @brief プレイヤーが近づくと吸い寄せられ始める距離（ユニット）
	constexpr float ATTRACT_RANGE{ 240.0f };

	/// @brief 吸い寄せられる速さ（ユニット/秒）
	constexpr float ATTRACT_SPEED{ 620.0f };

	/// @brief 取得と判定する水平距離（ユニット）
	constexpr float PICKUP_RANGE{ 90.0f };

	/// @brief 取得と判定する上下の許容差（ユニット）
	///
	/// 欠片は拾いやすいよう浮かせてあるので、真下に立ったときの縦の差を許容する。
	/// ここが足りないと、吸い寄せられて真上に来ているのに永久に拾えない
	constexpr float PICKUP_HEIGHT{ 180.0f };

	/// @brief 出現してから拾えるようになるまでの時間（秒）
	///
	/// 壊した勢いのまま即座に吸い込むと、何が出たのか見えないまま消える
	constexpr float PICKUP_DELAY{ 0.35f };
} // namespace

namespace game::system::stage
{
	ExtensionPickupSystem::ExtensionPickupSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::base::EventBus& eventBus,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	    , m_eventBus{ eventBus }
	    , m_playerId{ playerId }
	{
	}

	void ExtensionPickupSystem::update(float deltaTime)
	{
		const auto* playerTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(m_playerId) };

		const auto pickups{ m_componentManager.getAllEntities<component::stage::ExtensionPickupComponent>() };
		for (const auto id : pickups)
		{
			auto& pickup{ m_componentManager.get<component::stage::ExtensionPickupComponent>(id) };
			auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(id) };
			if (transform == nullptr)
				continue;

			pickup.m_elapsed += deltaTime;

			// プレイヤーとの距離は水平で測る。欠片は拾いやすいよう浮かせてあるので、
			// 3Dの直線距離で測ると真下に立っても縦の差だけで判定を外す
			const bool canInteract{ playerTransform != nullptr && pickup.m_elapsed >= PICKUP_DELAY };
			bool isAttracting{ false };
			if (canInteract)
			{
				const float dx{ playerTransform->m_position.x - transform->m_position.x };
				const float dz{ playerTransform->m_position.z - transform->m_position.z };
				isAttracting = dx * dx + dz * dz <= ATTRACT_RANGE * ATTRACT_RANGE;
			}

			if (isAttracting)
			{
				// 吸い寄せ中は落下も浮遊も止める。浮遊が毎フレームYを上書きすると、
				// プレイヤーの足元へ降りていけず永久に届かない
				pickup.m_isGrounded = true;
				pickup.m_velocity = {};

				const core::Vector3 toPlayer{ playerTransform->m_position - transform->m_position };
				if (toPlayer.lengthSq() > 0.0f)
					transform->m_position += toPlayer.normalized() * (ATTRACT_SPEED * deltaTime);
			}
			else if (!pickup.m_isGrounded)
			{
				transform->m_position += pickup.m_velocity * deltaTime;
				pickup.m_velocity.y -= GRAVITY * deltaTime;

				if (transform->m_position.y <= pickup.m_restY)
				{
					transform->m_position.y = pickup.m_restY;

					// 勢いが残っていれば一度だけ跳ねる。落ちて即止まると
					// 「置かれた」ように見えて、弾け出た感じが消える
					if (pickup.m_velocity.y < -LANDING_SPEED)
					{
						pickup.m_velocity.y = -pickup.m_velocity.y * BOUNCE_RETENTION;
						pickup.m_velocity.x *= BOUNCE_RETENTION;
						pickup.m_velocity.z *= BOUNCE_RETENTION;
					}
					else
					{
						pickup.m_isGrounded = true;
						pickup.m_velocity = {};
					}
				}
			}
			else
			{
				// 着地後はその場で上下に漂う。止まっていると背景に紛れる
				transform->m_position.y = pickup.m_restY +
				                          std::sin(pickup.m_elapsed * BOB_SPEED) * BOB_AMPLITUDE;
			}

			if (!canInteract)
				continue;

			// 取得判定も水平距離で見る。縦は「プレイヤーの身長ぶん」を許容して、
			// 浮いている欠片でも真下に立てば拾えるようにする
			const float dx{ playerTransform->m_position.x - transform->m_position.x };
			const float dy{ playerTransform->m_position.y - transform->m_position.y };
			const float dz{ playerTransform->m_position.z - transform->m_position.z };

			if (dx * dx + dz * dz <= PICKUP_RANGE * PICKUP_RANGE &&
			    std::abs(dy) <= PICKUP_HEIGHT)
			{
				m_eventBus.publish(event::ExtensionPickedUpEvent{ pickup.m_type });
				m_componentManager.removeAll(id);
				m_entityManager.destroy(core::ecs::Entity(id));
			}
		}
	}
} // namespace game::system::stage
