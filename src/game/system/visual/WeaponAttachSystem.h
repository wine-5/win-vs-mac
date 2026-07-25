#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"

namespace game::system::visual
{
	/**
	 * @brief 装着武器の「ボーン名 → フレーム番号」の解決を行うSystem
	 *
	 * 解決はEntityごとに一度だけ行い、以後は何もしない（毎フレームの検索を避ける）。
	 * 描画そのものは InGameView が担当する。
	 *
	 * 装着先が見つからなかった場合は、モデルが実際に持つボーン名を全て
	 * ログへ出力する。リグによってボーン名が異なるため、正しい名前を
	 * 突き止める手段をここに集約している。
	 */
	class WeaponAttachSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief WeaponAttachSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer ボーン検索に使うIRendererの参照
		 */
		WeaponAttachSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 未解決の装着武器について装着先ボーンを解決する
		 * @param deltaTime フレーム間の時間差（本Systemでは未使用）
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system::visual
