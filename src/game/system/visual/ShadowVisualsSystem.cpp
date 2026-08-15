#include "ShadowVisualsSystem.h"
#include <algorithm>
#include "core/utility/Color.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/visual/ShadowCasterComponent.h"

namespace
{
	/// @brief 影を地面からわずかに浮かせる量（床とのZファイティングを避ける）
	constexpr float GROUND_LIFT{ 2.0f };

	/// @brief コライダーの幅から影の半径を求める倍率
	///
	/// 幅そのままだと影が体より広く、浮いた輪に見える。少し内側へ締めると足元に収まる。
	constexpr float RADIUS_FROM_WIDTH{ 0.55f };

	/// @brief 影が完全に消えるまでの高さ
	///
	/// これだけ跳ぶと影は無くなる。ジャンプの最高到達点より少し高くしておくと、
	/// 通常のジャンプでは影が薄く小さくなるだけで消えず、位置の手掛かりが残る。
	constexpr float FADE_OUT_HEIGHT{ 600.0f };

	/// @brief 真下にいるとき（接地時）の影の濃さの倍率
	constexpr float MAX_ALPHA_SCALE{ 1.0f };

	/// @brief 高く跳んだときに影が縮む下限の倍率
	constexpr float MIN_SIZE_SCALE{ 0.4f };
} // namespace

namespace game::system::visual
{
	ShadowVisualsSystem::ShadowVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void ShadowVisualsSystem::update(float deltaTime)
	{
		(void)deltaTime;

		// 接地している間だけ足元の高さを覚える。ジャンプ中はこの値を使い続けることで、
		// 影が本体と一緒に浮き上がらず地面に残る
		for (const auto entityId :
		    m_componentManager.getAllEntities<component::visual::ShadowCasterComponent>())
		{
			const auto* velocity{
				m_componentManager.tryGet<component::movement::VelocityComponent>(entityId)
			};
			if (velocity == nullptr || !velocity->m_isGrounded)
				continue;

			const auto* transform{
				m_componentManager.tryGet<component::movement::TransformComponent>(entityId)
			};
			if (transform == nullptr)
				continue;

			auto& caster{ m_componentManager.get<component::visual::ShadowCasterComponent>(entityId) };
			caster.m_groundY = transform->m_position.y;
			caster.m_hasGroundY = true;
		}
	}

	void ShadowVisualsSystem::draw()
	{
		for (const auto entityId :
		    m_componentManager.getAllEntities<component::visual::ShadowCasterComponent>())
		{
			const auto& caster{ m_componentManager.get<component::visual::ShadowCasterComponent>(entityId) };

			// 一度も接地していないものは影を落とす地面が分からない（宙に浮く敵など）
			if (!caster.m_hasGroundY)
				continue;

			// 本体が消えているなら影も消す。死亡ディゾルブ中に影だけ残ると幽霊のように見える
			const auto* render{ m_componentManager.tryGet<component::visual::RenderComponent>(entityId) };
			if (render == nullptr || !render->m_isVisible)
				continue;

			const auto* transform{
				m_componentManager.tryGet<component::movement::TransformComponent>(entityId)
			};
			if (transform == nullptr)
				continue;

			// 半径の指定が無ければコライダーの幅から決める。どちらも無いものは描けない
			float radius{ caster.m_radius };
			if (radius <= 0.0f)
			{
				const auto* collider{
					m_componentManager.tryGet<component::combat::ColliderComponent>(entityId)
				};
				if (collider == nullptr)
					continue;
				radius = std::max(collider->m_size.x, collider->m_size.z) * RADIUS_FROM_WIDTH;
			}

			// 地面からの高さで薄く小さくする。真上から見た大きさは本来変わらないが、
			// 縮めたほうが「浮いている高さ」が直感的に読み取れる
			const float height{ std::max(0.0f, transform->m_position.y - caster.m_groundY) };
			const float fade{ 1.0f - std::min(1.0f, height / FADE_OUT_HEIGHT) };
			if (fade <= 0.0f)
				continue;

			const float sizeScale{ MIN_SIZE_SCALE + (1.0f - MIN_SIZE_SCALE) * fade };

			const auto baseAlpha{ (core::utility::Color::GROUND_SHADOW >> 24) & 0xFFu };
			const auto alpha{ static_cast<unsigned int>(baseAlpha * fade * MAX_ALPHA_SCALE) };
			const unsigned int color{ (alpha << 24) | (core::utility::Color::GROUND_SHADOW & 0x00FFFFFFu) };

			core::Vector3 center{ transform->m_position };
			center.y = caster.m_groundY + GROUND_LIFT;

			m_renderer.drawGroundCircle(center, radius * sizeScale, color, true);
		}
	}
} // namespace game::system::visual
