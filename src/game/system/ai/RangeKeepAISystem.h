#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"

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
		core::ecs::ComponentManager& m_componentManager;
		float m_elapsedTime{ 0.0f }; // ホバーの揺らぎ（アイドルの上下動）の位相計算用の経過時間
	};
} // namespace game::system::ai
