#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IProjectileWindowManager.h"

namespace game::system
{
	/**
	 * @brief 弾の位置に実OSウィンドウを追従させるSystem
	 *
	 * 弾エンティティ（当たり判定・移動の真）のワールド座標をスクリーン座標へ射影し、
	 * IProjectileWindowManager（Platform層）に配置情報として渡す。
	 * 画面に映っていない弾（カメラ後方など）はウィンドウを出さない。
	 */
	class ProjectileWindowSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief ProjectileWindowSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer ワールド→スクリーン変換に使う描画インターフェース
		 * @param windowManager 実ウィンドウの表示・移動を担うマネージャ
		 */
		ProjectileWindowSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer,
		    core::iface::IProjectileWindowManager& windowManager);

		/**
		 * @brief 全弾のスクリーン位置を計算してウィンドウ群を追従させる
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IProjectileWindowManager& m_windowManager;
	};
} // namespace game::system
