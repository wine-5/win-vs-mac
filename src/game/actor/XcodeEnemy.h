#pragma once
#include "game/actor/EnemyBase.h"

namespace game::actor
{
	/**
	 * @brief 近接雑魚敵のXcode
	 *
	 * 近接追跡型：プレイヤーを追跡し、GroundSlamで近接攻撃する
	 */
	class XcodeEnemy : public EnemyBase
	{
	public:
		using EnemyBase::EnemyBase; // 基底のコンストラクタをそのまま継承する

	protected:
		/**
		 * @brief Xcodeのアニメーションクリップを登録する
		 */
		void setupAnimation() override;

		/**
		 * @brief 近接追跡型のAI設定を行う
		 */
		void setupAI() override;
	};
} // namespace game::actor