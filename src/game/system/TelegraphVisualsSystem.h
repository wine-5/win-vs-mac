#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"

namespace game::system
{
	/**
	 * @brief TelegraphComponentを持つ敵の攻撃予兆（円・扇）を地面に描くSystem
	 *
	 * ボス等が溜め（ウィンドアップ）中に TelegraphComponent へ書いた形状・範囲・進行度を
	 * 読み、危険範囲を地面へ予告表示する。進行度に応じて内側が中心から満ちていき、
	 * 満ちきった瞬間に攻撃が発動する。状態はComponent側が持つため更新処理は無し。
	 * drawはInGameViewの描画フェーズ（3D描画中）から呼ばれる。
	 * 近接ワインドアップ専用の AttackTelegraphVisualsSystem とは別に、任意形状を扱う。
	 */
	class TelegraphVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 地面への円・扇描画に使うIRenderer
		 */
		TelegraphVisualsSystem(core::ecs::ComponentManager& componentManager, core::iface::IRenderer& renderer);

		/**
		 * @brief 更新処理（状態はTelegraphComponentが持つため何もしない）
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief アクティブな予兆を地面に描く（描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system
