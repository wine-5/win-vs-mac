#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/utility/Vector3.h"
#include <random>

namespace game::component
{
	namespace movement
	{
		struct TransformComponent;
	}
	namespace ai
	{
		struct RangeKeepAIComponent;
	}
} // namespace game::component

namespace game::system::ai
{
	/**
	 * @brief 遠距離維持型AIを駆動するSystem
	 *
	 * RangeKeepAIComponentを持つ敵に対して毎フレーム処理を実行する。
	 * プレイヤーから一定距離（preferredDistanceMin/Max）を保つように移動する。
	 * 推奨距離より遠ければ接近、近ければ後退する。
	 * ホバー高度（hoverHeight）が指定されていればその高さを保つ。
	 */
	class RangeKeepAISystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief RangeKeepAISystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		explicit RangeKeepAISystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief RangeKeepAIComponentを持つすべての敵の距離維持処理を実行する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief 索敵範囲外の徘徊（ホーム周辺をふらつく）を1体分処理する
		 *
		 * 水平方向はホーム周辺の目的地へゆっくり移動し、到着したら少し待機する。
		 * 浮遊高度（＋アイドル揺らぎ）は移動中・待機中どちらも維持する。
		 * @param entityId 対象のEntityId
		 * @param rangeKeep 対象のRangeKeepAIComponent（徘徊状態を保持する）
		 * @param transform 対象のTransform
		 * @param deltaTime フレーム間の時間差
		 */
		void updatePatrol(core::ecs::EntityId entityId, component::ai::RangeKeepAIComponent& rangeKeep,
		    component::movement::TransformComponent& transform, float deltaTime);

		/**
		 * @brief ホーム地点まわりの水平な徘徊目的地をランダムに選ぶ
		 * @param home 徘徊の基準点（スポーン地点）
		 * @return 選ばれた目的地（yはhomeのまま。水平のみ動かす）
		 */
		core::Vector3 pickWanderTarget(const core::Vector3& home);

		core::ecs::ComponentManager& m_componentManager;
		float m_elapsedTime{ 0.0f };                  // ホバーの揺らぎ（アイドルの上下動）の位相計算用の経過時間
		std::mt19937 m_rng{ std::random_device{}() }; // 徘徊目的地・待機時間の乱数エンジン
	};
} // namespace game::system::ai
