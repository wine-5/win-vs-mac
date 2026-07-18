#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"

namespace game::system::ai
{
	/**
	 * @brief 近接追跡型AIを駆動するSystem
	 *
	 * MeleeChaseAIComponentを持つ敵に対して毎フレーム処理を実行する。
	 * AIComponentの共通フィールド（targetEntity・detectionRange・moveSpeed等）を
	 * 読み込み、プレイヤーへの移動・回転・アニメ要求・攻撃判定を書き込む。
	 * 攻撃はレンジ内でのみ要求される（距離判定あり）。
	 */
	class MeleeChaseAISystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief MeleeChaseAISystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		explicit MeleeChaseAISystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief MeleeChaseAIComponentを持つすべての敵の近接追跡処理を実行する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system::ai
