#include "EnemyHealthBarView.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "game/component/EnemyTypeComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/DeathComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/movement/TransformComponent.h"
#include <algorithm>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// バーの見た目（1080p基準）
	constexpr int BAR_WIDTH{ 84 };
	constexpr int BAR_HEIGHT{ 8 };
	constexpr int HEAD_MARGIN{ 40 }; // 頭のてっぺんからバーまでの間隔（ワールド単位）

	constexpr int BAR_GROOVE_ALPHA{ 150 }; // 溝は暗く敷く（明るい背景でも輪郭が出るように）
	constexpr unsigned int BAR_GROOVE_COLOR{ 0xFF0E1420 };
	constexpr unsigned int BAR_FILL_COLOR{ 0xFFE81123 }; // 敵＝赤。プレイヤーの緑と取り違えない

	// コライダーが無い敵向けの頭の高さ（ワールド単位）
	constexpr float FALLBACK_HEAD_HEIGHT{ 150.0f };
} // namespace

namespace game::ui::ingame
{
	EnemyHealthBarView::EnemyHealthBarView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	int EnemyHealthBarView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void EnemyHealthBarView::draw(core::ecs::EntityId bossId)
	{
		// 敵種を持つEntityだけを対象にする。HealthComponentで走査するとプレイヤーまで拾ってしまう
		for (const auto entityId : m_componentManager.getAllEntities<component::EnemyTypeComponent>())
		{
			// ボスは上中央に専用のHUDがある
			if (entityId == bossId)
				continue;

			// 死亡ディゾルブ中はバーを消す。消えかけの敵に残りHPを出しても意味がない
			if (m_componentManager.has<component::combat::DeathComponent>(entityId))
				continue;

			if (!m_componentManager.has<component::combat::HealthComponent>(entityId))
				continue;
			if (!m_componentManager.has<component::movement::TransformComponent>(entityId))
				continue;

			const auto& health{ m_componentManager.get<component::combat::HealthComponent>(entityId) };
			if (health.m_maxHp <= 0.0f || health.m_currentHp <= 0.0f)
				continue;

			// 無傷の敵には出さない。全員に出すと画面がバーだらけになり、
			// 交戦中の相手がかえって埋もれる
			if (health.m_currentHp >= health.m_maxHp)
				continue;

			drawBarAboveHead(entityId, std::clamp(health.m_currentHp / health.m_maxHp, 0.0f, 1.0f));
		}
	}

	void EnemyHealthBarView::drawBarAboveHead(core::ecs::EntityId entityId, float ratio)
	{
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };

		// 頭上のワールド座標を求める（原点は足元。コライダー高さぶん上へ）
		float headHeight{ FALLBACK_HEAD_HEIGHT };
		if (m_componentManager.has<component::combat::ColliderComponent>(entityId))
			headHeight = m_componentManager.get<component::combat::ColliderComponent>(entityId).m_size.y;

		core::Vector3 headWorld{ transform.m_position };
		headWorld.y += headHeight + HEAD_MARGIN;

		// スクリーン座標へ投影。カメラ後方・画面外（深度が0〜1の外）なら描かない
		const core::Vector3 screen{ m_renderer.worldToScreen(headWorld) };
		if (screen.z < 0.0f || screen.z > 1.0f)
			return;

		const int width{ scaled(BAR_WIDTH) };
		const int height{ scaled(BAR_HEIGHT) };
		const int radius{ height / 2 };
		const int x{ static_cast<int>(screen.x) - width / 2 };
		const int y{ static_cast<int>(screen.y) - height };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BAR_GROOVE_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, BAR_GROOVE_COLOR, true, 1);
		m_uiRenderer.resetBlendMode();

		// 幅が高さを下回るとピルが成立しないため、残量が僅かでも高さぶんは確保する
		const int fillWidth{ std::max(height, static_cast<int>(width * ratio)) };
		m_uiRenderer.drawRoundedBox(x, y, fillWidth, height, radius, BAR_FILL_COLOR, true, 1);
	}
} // namespace game::ui::ingame
