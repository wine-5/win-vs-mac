#include "ProjectileWindowSystem.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/AttackComponent.h"
#include <cmath>

namespace
{
	constexpr int MIN_WINDOW_SIZE{ 24 };  // 遠くの弾でも視認できる最小ピクセルサイズ
	constexpr int MAX_WINDOW_SIZE{ 512 }; // 近すぎる弾で巨大化しすぎないための上限
} // namespace

namespace game::system
{
	ProjectileWindowSystem::ProjectileWindowSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer,
	    core::iface::IProjectileWindowManager& windowManager)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	    , m_windowManager{ windowManager }
	{
	}

	void ProjectileWindowSystem::update(float deltaTime)
	{
		std::vector<core::iface::ProjectileWindowInfo> infos{};

		auto projectiles{ m_componentManager.getAllEntities<component::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			if (!m_componentManager.has<component::TransformComponent>(id))
				continue;

			const auto& transform{ m_componentManager.get<component::TransformComponent>(id) };

			// 弾の中心をスクリーンへ射影する。z が 0〜1 の範囲外なら画面に映っていない
			const core::Vector3 center{ m_renderer.worldToScreen(transform.m_position) };
			if (center.z <= 0.0f || center.z >= 1.0f)
				continue;

			// 見た目サイズ：中心と「半径ぶん上の点」の射影距離から画面上の大きさを求める
			float radius{ 40.0f };
			if (m_componentManager.has<component::AttackComponent>(id))
				radius = m_componentManager.get<component::AttackComponent>(id).m_attackRange;

			const core::Vector3 topWorld{
				transform.m_position.x,
				transform.m_position.y + radius,
				transform.m_position.z
			};
			const core::Vector3 top{ m_renderer.worldToScreen(topWorld) };

			int size{ static_cast<int>(std::abs(center.y - top.y) * 2.0f) };
			if (size < MIN_WINDOW_SIZE)
				size = MIN_WINDOW_SIZE;
			if (size > MAX_WINDOW_SIZE)
				size = MAX_WINDOW_SIZE;

			core::iface::ProjectileWindowInfo info{};
			info.m_centerX = static_cast<int>(center.x);
			info.m_centerY = static_cast<int>(center.y);
			info.m_size = size;
			infos.push_back(info);
		}

		// 空リストなら全ウィンドウが隠れる
		m_windowManager.updateWindows(infos);
	}
} // namespace game::system
