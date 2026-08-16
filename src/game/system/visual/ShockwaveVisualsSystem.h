#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"

namespace game::system::visual
{
	/**
	 * @brief ShockwaveComponentを持つEntityの位置へ、広がる衝撃波の輪を描くSystem
	 *
	 * 画面のシェイクだけでは揺れた原因が分からないため、その原因を地面に見せる。
	 * drawはInGameViewの描画フェーズ（3D描画中）から呼ばれる。
	 */
	class ShockwaveVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief ShockwaveVisualsSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 地面への円描画に使うIRenderer
		 */
		ShockwaveVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 再生中の衝撃波の時間を進め、終わったものを止める
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/** @brief 再生中の衝撃波を地面へ描く（描画フェーズから呼ぶ） */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system::visual
