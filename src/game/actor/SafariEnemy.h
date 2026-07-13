#pragma once
#include "game/actor/EnemyBase.h"

namespace game::actor
{
	/**
	 * @brief 遠距離雑魚敵のSafari
	 *
	 * 距離維持型：浮遊しながらプレイヤーと一定距離を保ち、弾を投げる。
	 * アニメーションを持たず、動きはコード制御（スケール変化等）で表現する
	 */
	class SafariEnemy : public EnemyBase
	{
	public:
		using EnemyBase::EnemyBase; // 基底のコンストラクタをそのまま継承する

	protected:
		/**
		 * @brief アニメーションなしのため何もしない
		 */
		void setupAnimation() override;

		/**
		 * @brief 距離維持型のAI設定を行う
		 */
		void setupAI() override;
	};
}