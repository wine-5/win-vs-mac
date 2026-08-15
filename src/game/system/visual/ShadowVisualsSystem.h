#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"

namespace game::system::visual
{
	/**
	 * @brief ShadowCasterComponentを持つEntityの足元へ接地影を描くSystem
	 *
	 * プレイヤーや敵が「床の上に立っている」ことを示すための影。虚無の背景に
	 * 明るい床という画作りでは、影が無いとキャラクターが浮いて見える。
	 *
	 * 影は真下へ落とす円で、光源の向きに合わせて伸ばしたりはしない。
	 * ジャンプ中も影を地面に残すため、接地していた高さをComponentへ覚えておき、
	 * 高く跳ぶほど薄く小さくする（距離感の手掛かりになる）。
	 * drawはInGameViewの描画フェーズ（3D描画中）から呼ばれる。
	 */
	class ShadowVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief ShadowVisualsSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 地面への円描画に使うIRenderer
		 */
		ShadowVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 接地しているEntityの足元の高さを記録する
		 * @param deltaTime フレーム間の時間差（未使用）
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 影を落とすEntityの足元へ円を描く（描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system::visual
