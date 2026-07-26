#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/base/EventBus.h"
#include <vector>

namespace game::system::stage
{
	/**
	 * @brief ボス出現で入り口の扉をせり上げて塞ぐSystem
	 *
	 * BossAppearedEvent を購読し、BossGateComponentを持つ配置物を床下（開）から
	 * ステージJSONに置かれた高さ（閉）まで持ち上げる。
	 * 当たり判定はStagePropのBoxコライダーがTransformに追従するため、
	 * せり上がりきればそのまま通せんぼになる。
	 */
	class BossGateSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief BossGateSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus BossAppearedEvent購読用のEventBus
		 */
		BossGateSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus);

		/**
		 * @brief 閉じ動作を進め、扉のY座標を更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;

		// ボス出現イベントを受け取ったか。イベント中にECSを触らず、次のupdateで反映する
		bool m_isTriggered{ false };

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::stage
