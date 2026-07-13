#pragma once
#include "game/actor/EnemyBase.h"

namespace game::actor
{
	/**
	 * @brief Boss敵のMac
	 *
	 * ボス型：接近、遠距離などを使い分ける
	 */
	class MacEnemy : public EnemyBase
	{
	public:
		using EnemyBase::EnemyBase; // 基底のコンストラクタをそのまま継承する

	protected:
		/**
		 * @brief Macのアニメーションクリップを登録する
		 */
		void setupAnimation() override;

		/**
		 * @brief ボス型のAI設定を行う
		 */
		void setupAI() override;
	};
}