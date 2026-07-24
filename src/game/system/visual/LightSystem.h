#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include <unordered_map>
#include <vector>

namespace core::iface
{
	class ILighting;
}

namespace game::system::visual
{
	/**
	 * @brief LightComponent を持つエンティティの点光源を生成・追従させるSystem
	 *
	 * 生成した光源ハンドルはComponent側に保持し、エンティティが消えたら破棄する。
	 * 同時に有効にできる光源数には上限があるため、光源は必要な箇所だけに置くこと。
	 */
	class LightSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief LightSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param lighting ライティング操作インターフェースの参照
		 */
		LightSystem(core::ecs::ComponentManager& componentManager, core::iface::ILighting& lighting);

		/** @brief 生成済みの光源をすべて破棄する */
		~LightSystem() override;

		/**
		 * @brief 光源の生成と位置の追従を行う
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief 消滅したEntityの光源を破棄する
		 * @param aliveEntities 今フレームに存在したLightComponent保持Entityの一覧
		 */
		void destroyLightsOfLostEntities(const std::vector<core::ecs::EntityId>& aliveEntities);

		core::ecs::ComponentManager& m_componentManager;
		core::iface::ILighting& m_lighting;

		// 生成した光源を持ち主のEntityと対応づけて保持する。
		// 敵が倒されてEntityごと消えたとき、これを見て光源を破棄する
		// （放置すると死んだ場所に光が残り続け、召喚のたびに蓄積する）
		std::unordered_map<core::ecs::EntityId, int> m_lightsByEntity;
	};
} // namespace game::system::visual
