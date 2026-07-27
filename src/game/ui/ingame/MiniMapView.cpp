#include "MiniMapView.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/GroundSurfaceComponent.h"
#include "game/component/camera/CameraComponent.h"
#include "game/utility/MiniMapProjection.h"
#include <array>
#include <cmath>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// マップの位置と大きさ（右上・1080p基準）
	constexpr int MAP_MARGIN{ 28 };
	constexpr int MAP_SIZE{ 200 };
	constexpr int MAP_CORNER_RADIUS{ 8 }; // Windows 11のパネルと同じ角丸

	// マップの端に映るワールド距離（ユニット）。狭いと索敵に使えず、広いと潰れる
	constexpr float MAP_RANGE{ 4000.0f };

	// 自機の高さからこれだけ離れた床は薄く描き、倍を超えたら描かない。
	// 坂ぶんの上下は残しつつ、別階層の床は拾わない値にする
	constexpr float LAYER_TOLERANCE{ 700.0f };

	// 床の塗りと輪郭の不透明度
	constexpr int FLOOR_FILL_ALPHA{ 76 };   // 約30%
	constexpr int FLOOR_LINE_ALPHA{ 216 };  // 約85%
	constexpr float FAR_LAYER_DIM{ 0.33f }; // 高さが離れた床を薄める割合

	// パネルの上にもう一段沈める暗幕。地形の線が背景に埋もれないようにする
	constexpr unsigned int MAP_BACKGROUND{ 0xFF060A12 };
	constexpr int MAP_BACKGROUND_ALPHA{ 128 };

	// 自機の矢印（1080p基準）
	constexpr int ARROW_HALF_HEIGHT{ 7 };
	constexpr int ARROW_HALF_WIDTH{ 5 };
	constexpr int ARROW_TAIL{ 3 }; // 後端のくびれ

	// 床の色。配置物の種類（stageCatalog.jsonのid）はゲーム層まで降りてこないため、
	// GroundSurfaceComponentが持つ「振る舞い」で塗り分ける。
	// プレイヤーにとっても見た目より「滑るのか・運ばれるのか」のほうが役に立つ
	constexpr unsigned int COLOR_FLOOR{ 0xFF0067C0 };    // 普通の足場
	constexpr unsigned int COLOR_SLOPE{ 0xFF3FB950 };    // 滑る坂
	constexpr unsigned int COLOR_CONVEYOR{ 0xFF00B7C3 }; // 動く歩道
} // namespace

namespace game::ui::ingame
{
	MiniMapView::MiniMapView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_panel{ uiRenderer, screen }
	{
	}

	int MiniMapView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void MiniMapView::drawRotatedRect(float centerX, float centerY, float halfWidth, float halfDepth,
	    float angle, unsigned int color, int fillAlpha, int lineAlpha)
	{
		const float sinA{ std::sin(angle) };
		const float cosA{ std::cos(angle) };

		// 4隅を回して求める。順序は左上→右上→右下→左下（輪郭を一筆で描ける並び）
		constexpr int CORNER_COUNT{ 4 };
		const std::array<std::pair<float, float>, CORNER_COUNT> localCorners{ {
			{ -halfWidth, -halfDepth },
			{ halfWidth, -halfDepth },
			{ halfWidth, halfDepth },
			{ -halfWidth, halfDepth },
		} };

		std::array<int, CORNER_COUNT> screenX{};
		std::array<int, CORNER_COUNT> screenY{};
		for (int i{ 0 }; i < CORNER_COUNT; ++i)
		{
			const auto& [lx, ly]{ localCorners[i] };
			screenX[i] = static_cast<int>(centerX + lx * cosA - ly * sinA);
			screenY[i] = static_cast<int>(centerY + lx * sinA + ly * cosA);
		}

		// 塗りは三角形2枚。矩形を回して描く手段が三角形しかないため
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, fillAlpha);
		m_uiRenderer.drawTriangle(screenX[0], screenY[0], screenX[1], screenY[1],
		    screenX[2], screenY[2], color, true);
		m_uiRenderer.drawTriangle(screenX[0], screenY[0], screenX[2], screenY[2],
		    screenX[3], screenY[3], color, true);

		// 輪郭。隣り合う床が1枚の塊に見えないよう縁を立てる
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, lineAlpha);
		for (int i{ 0 }; i < CORNER_COUNT; ++i)
		{
			const int next{ (i + 1) % CORNER_COUNT };
			m_uiRenderer.drawLine(screenX[i], screenY[i], screenX[next], screenY[next], color, 1);
		}
		m_uiRenderer.resetBlendMode();
	}

	void MiniMapView::drawFloors(int centerX, int centerY, const core::Vector3& playerPosition,
	    float yaw, float scale, int radius)
	{
		const auto surfaces{ m_componentManager.getAllEntities<component::movement::GroundSurfaceComponent>() };

		for (const auto surfaceId : surfaces)
		{
			const auto& surface{ m_componentManager.get<component::movement::GroundSurfaceComponent>(surfaceId) };
			const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(surfaceId) };

			// 天面の高さで階層を判定する。自機から離れているほど薄くし、離れすぎたら描かない
			const float top{ transform.m_position.y + surface.m_size.y * 0.5f };
			const float heightDiff{ std::abs(top - playerPosition.y) };
			if (heightDiff > LAYER_TOLERANCE * 2.0f)
				continue;
			const float dim{ heightDiff > LAYER_TOLERANCE ? FAR_LAYER_DIM : 1.0f };

			const auto point{ utility::projectToMiniMap(transform.m_position.x, transform.m_position.z,
				playerPosition.x, playerPosition.z, yaw, scale) };

			const float halfWidth{ surface.m_size.x * 0.5f * scale };
			const float halfDepth{ surface.m_size.z * 0.5f * scale };

			// 枠から完全に外れた床は描かない。床は数十枚あるので早めに捨てる
			const float distance{ std::sqrt(point.m_x * point.m_x + point.m_y * point.m_y) };
			if (distance > radius + std::max(halfWidth, halfDepth))
				continue;

			unsigned int color{ COLOR_FLOOR };
			if (surface.m_conveyorSpeed != 0.0f)
				color = COLOR_CONVEYOR;
			else if (surface.m_slideAccel > 0.0f)
				color = COLOR_SLOPE;

			drawRotatedRect(centerX + point.m_x, centerY + point.m_y, halfWidth, halfDepth,
			    utility::projectYawToMiniMap(transform.m_rotation.y, yaw), color,
			    static_cast<int>(FLOOR_FILL_ALPHA * dim),
			    static_cast<int>(FLOOR_LINE_ALPHA * dim));
		}
	}

	void MiniMapView::drawPlayerArrow(int centerX, int centerY)
	{
		// 回転式なので自機は常に中心・常に上向き。矢印そのものは回さない
		const int halfHeight{ scaled(ARROW_HALF_HEIGHT) };
		const int halfWidth{ scaled(ARROW_HALF_WIDTH) };
		const int tail{ scaled(ARROW_TAIL) };

		m_uiRenderer.drawTriangle(centerX, centerY - halfHeight,
		    centerX + halfWidth, centerY + halfHeight, centerX, centerY + tail,
		    core::utility::Color::HUD_INK, true);
		m_uiRenderer.drawTriangle(centerX, centerY - halfHeight,
		    centerX - halfWidth, centerY + halfHeight, centerX, centerY + tail,
		    core::utility::Color::HUD_INK, true);
	}

	void MiniMapView::draw(core::ecs::EntityId playerId)
	{
		if (!m_componentManager.has<component::movement::TransformComponent>(playerId))
			return;

		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(playerId) };

		// 向きはモデルの回転ではなくカメラのyawを使う。プレイヤーが見ている方向を上にしたいため
		float yaw{ 0.0f };
		if (const auto* camera{ m_componentManager.tryGet<component::camera::CameraComponent>(playerId) })
			yaw = camera->m_yaw;

		const int size{ scaled(MAP_SIZE) };
		const int mapX{ m_screen.getWidth() - scaled(MAP_MARGIN) - size };
		const int mapY{ scaled(MAP_MARGIN) };
		const int centerX{ mapX + size / 2 };
		const int centerY{ mapY + size / 2 };
		const int radius{ size / 2 };
		const float scale{ static_cast<float>(radius) / MAP_RANGE };

		// 下地は他のHUDと同じサーフェス。その上にもう一段沈めて地形の線を立たせる
		m_panel.draw(mapX, mapY, size, size);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, MAP_BACKGROUND_ALPHA);
		m_uiRenderer.drawRoundedBox(mapX, mapY, size, size, scaled(MAP_CORNER_RADIUS),
		    MAP_BACKGROUND, true, 1);
		m_uiRenderer.resetBlendMode();

		// ここから枠の内側だけに描く。解除を忘れると以降のHUDがすべて消える
		m_uiRenderer.setClipArea(mapX, mapY, size, size);
		drawFloors(centerX, centerY, transform.m_position, yaw, scale, radius);
		m_uiRenderer.resetClipArea();

		drawPlayerArrow(centerX, centerY);
	}
} // namespace game::ui::ingame
