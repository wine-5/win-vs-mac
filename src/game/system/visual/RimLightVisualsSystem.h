#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"

namespace game::system::visual
{
	/**
	 * @brief RimLightComponentを持つEntityの輪郭を光らせるSystem
	 *
	 * 少し膨らませたモデルの裏面だけを発光色で描き、本体からはみ出した分を
	 * 輪郭として見せる。本体はこの直後に InGameView が通常描画する。
	 *
	 * 明るい床に暗いキャラクターという画作りのため、輪郭が無いと背景に溶ける。
	 * drawはInGameViewの描画フェーズ（モデル描画の直前）から呼ばれる。
	 */
	class RimLightVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief RimLightVisualsSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 輪郭描画に使うIRenderer
		 */
		RimLightVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 更新処理（状態を持たないため何もしない）
		 */
		void update(float /*deltaTime*/) override;

		/**
		 * @brief 対象Entityの輪郭を描く（描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system::visual
