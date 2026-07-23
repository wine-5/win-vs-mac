#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include "game/constant/AnimationState.h"
#include <random>

namespace game::system::ai
{
	/**
	 * @brief 近接追跡型AIを駆動するSystem
	 *
	 * MeleeChaseAIComponentを持つ敵に対して毎フレーム処理を実行する。
	 * プレイヤーが索敵範囲内なら追跡（Chase）してレンジ内で攻撃し、
	 * 範囲外ならスポーン地点まわりを徘徊（Patrol）する状態機械として動く。
	 * AIComponentの共通フィールド（targetEntity・detectionRange・moveSpeed等）を読み、
	 * 移動・回転・アニメ要求・攻撃判定を書き込む。
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
		/**
		 * @brief 追跡状態の処理：プレイヤーへ接近し、攻撃レンジ内で止まって攻撃する
		 * @param entityId 対象のEntityId
		 * @param distanceToPlayer プレイヤーまでの水平距離
		 * @param dirToPlayer プレイヤーへの正規化済み水平方向ベクトル
		 * @param deltaTime フレーム間の時間差
		 */
		void updateChase(core::ecs::EntityId entityId, float distanceToPlayer, const core::Vector3& dirToPlayer, float deltaTime);

		/**
		 * @brief 巡回状態の処理：スポーン地点まわりを徘徊し、たまに立ち止まる
		 * @param entityId 対象のEntityId
		 * @param deltaTime フレーム間の時間差
		 */
		void updatePatrol(core::ecs::EntityId entityId, float deltaTime);

		/**
		 * @brief 徘徊の目的地をスポーン地点まわりからランダムに選ぶ
		 * @param home スポーン地点（徘徊の基準）
		 * @return 選ばれた目的地のワールド座標
		 */
		core::Vector3 pickWanderTarget(const core::Vector3& home);

		/**
		 * @brief アニメーション状態を要求する（AnimationComponentを持つ場合のみ）
		 * @param entityId 対象のEntityId
		 * @param state 要求する状態
		 */
		void requestAnimation(core::ecs::EntityId entityId, constant::AnimationState state);

		core::ecs::ComponentManager& m_componentManager;
		std::mt19937 m_rng{ std::random_device{}() };
	};
} // namespace game::system::ai
