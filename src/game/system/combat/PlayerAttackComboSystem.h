#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system::combat
{
	/**
	 * @brief プレイヤーの近接攻撃入力を受け取り、コンボの段数へ振り分けるSystem
	 *
	 * 1段目（斬り）を出してから受付時間内に押し直すと2段目（回転斬り）へ進み、
	 * 受付時間を過ぎていれば1段目へ戻る。2段で打ち止めで、3段目には進まない。
	 *
	 * 攻撃の成立判定（クールダウン・ダメージ解決）は AttackSystem が持つ。
	 * 本Systemは「入力をどの段の攻撃として扱うか」だけを決め、攻撃要求と
	 * アニメーション要求を出す。
	 */
	class PlayerAttackComboSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief PlayerAttackComboSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param playerId プレイヤーのEntityID
		 */
		PlayerAttackComboSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 受付時間を消化し、攻撃入力があれば段数を進めて要求を出す
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;

		// 押しっぱなしで段が進まないよう、押した瞬間だけを拾うための前フレームの状態
		bool m_wasAttackPressed{ false };
	};
} // namespace game::system::combat
