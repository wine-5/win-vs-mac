#include "DebugHUDView.h"
#include "core/interface/IPerformanceDataProvider.h"
#include "core/interface/IEffectFactory.h" // DEBUG: リリース時に削除
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include <cstdio>

namespace
{
	// 右上に統計を表示する際のフォントサイズ・行間・余白
	constexpr int STATS_FONT_SIZE{ 22 };
	constexpr int STATS_LINE_HEIGHT{ 26 };
	constexpr int STATS_MARGIN{ 16 };
	constexpr unsigned int STATS_TEXT_COLOR{ 0xFFFFFF00 }; // 黄色（ARGB）
} // namespace

namespace game::ui::debug
{
	DebugHUDView::DebugHUDView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager,
	    GameManager& gameManager,
	    PauseManager& pauseManager,
	    core::iface::IPerformanceDataProvider& perfProvider,
	    core::iface::IEffectFactory& effectFactory)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_gameManager{ gameManager }
	    , m_pauseManager{ pauseManager }
	    , m_perfProvider{ perfProvider }
	    , m_effectFactory{ effectFactory }
	{
	}

	void DebugHUDView::update()
	{
		// 実際の壁時計時間を計測する（Applicationの固定タイムステップは使わない。
		// ヘッダのコメント参照）
		const auto now{ std::chrono::steady_clock::now() };
		if (!m_hasLastUpdateTime)
		{
			m_lastUpdateTime = now;
			m_hasLastUpdateTime = true;
			return;
		}
		const float realDeltaTime{ std::chrono::duration<float>(now - m_lastUpdateTime).count() };
		m_lastUpdateTime = now;

		// FPS/フレーム時間は直近FPS_UPDATE_INTERVAL秒間のフレーム数から算出する
		// （毎フレームの瞬間値だと数値が激しく揺れて読みにくいため）
		++m_fpsFrameAccum;
		m_fpsTimeAccum += realDeltaTime;
		if (m_fpsTimeAccum >= FPS_UPDATE_INTERVAL)
		{
			m_displayFps = static_cast<float>(m_fpsFrameAccum) / m_fpsTimeAccum;
			m_displayFrameMs = (m_fpsTimeAccum / static_cast<float>(m_fpsFrameAccum)) * 1000.0f;
			m_fpsFrameAccum = 0;
			m_fpsTimeAccum = 0.0f;
		}

		// CPU/メモリ使用率は重い処理を伴うため一定間隔でのみ更新する
		m_perfUpdateTimer += realDeltaTime;
		if (m_perfUpdateTimer >= PERF_UPDATE_INTERVAL)
		{
			m_perfUpdateTimer -= PERF_UPDATE_INTERVAL;
			m_perfProvider.update();
		}
	}

	void DebugHUDView::draw(int enemyCount)
	{
		drawCameraLabel();
		drawStats(enemyCount);
	}

	void DebugHUDView::drawCameraLabel()
	{
		const bool isSceneView{ m_pauseManager.isPausedBy(PauseReason::DebugSceneView) };
		if (!m_gameManager.isDebugMode() && !isSceneView)
			return;

		constexpr int LABEL_X{ 16 };
		constexpr int LABEL_Y{ 16 };
		constexpr int FONT_SIZE{ 28 };
		constexpr unsigned int TEXT_COLOR{ 0xFFFFFF00 }; // 黄色（ARGB）

		m_uiRenderer.drawText(LABEL_X, LABEL_Y,
		    isSceneView ? "SceneView (Time Stopped)" : "DebugCamera",
		    TEXT_COLOR, FONT_SIZE);

		// 操作方法を併記する（WASD=カメラ、矢印キー=Player）
		m_uiRenderer.drawText(LABEL_X, LABEL_Y + FONT_SIZE + 4,
		    isSceneView ? "WASD/Space/Shift: Camera" : "WASD/Space/Shift: Camera   Arrows: Player",
		    TEXT_COLOR, FONT_SIZE);
	}

	void DebugHUDView::drawStats(int enemyCount)
	{
		const int entityCount{ static_cast<int>(m_componentManager.getAllEntities<component::movement::TransformComponent>().size()) };
		const int projectileCount{ static_cast<int>(m_componentManager.getAllEntities<component::combat::ProjectileComponent>().size()) };
		const int activeEffectCount{ m_effectFactory.getActiveEffectCount() };
		const auto snapshot{ m_perfProvider.getSnapshot() };

		char lines[6][64]{};
		std::snprintf(lines[0], sizeof(lines[0]), "FPS: %.1f (%.2fms)", m_displayFps, m_displayFrameMs);
		std::snprintf(lines[1], sizeof(lines[1]), "Entity: %d", entityCount);
		std::snprintf(lines[2], sizeof(lines[2]), "Enemy: %d  Bullet: %d", enemyCount, projectileCount);
		std::snprintf(lines[3], sizeof(lines[3]), "Active Effects: %d", activeEffectCount);
		std::snprintf(lines[4], sizeof(lines[4]), "System  CPU: %.1f%%  Mem: %.1f%%", snapshot.cpuUsage * 100.0f, snapshot.memoryUsage * 100.0f);
		std::snprintf(lines[5], sizeof(lines[5]), "This Game  CPU: %.1f%%  Mem: %.0fMB", snapshot.processCpuUsage * 100.0f, snapshot.processMemoryUsageMB);

		for (int i{ 0 }; i < 6; ++i)
		{
			const int textWidth{ m_uiRenderer.getTextWidth(lines[i], STATS_FONT_SIZE) };
			const int x{ m_screen.getWidth() - textWidth - STATS_MARGIN };
			const int y{ STATS_MARGIN + i * STATS_LINE_HEIGHT };
			m_uiRenderer.drawText(x, y, lines[i], STATS_TEXT_COLOR, STATS_FONT_SIZE);
		}
	}
} // namespace game::ui::debug
