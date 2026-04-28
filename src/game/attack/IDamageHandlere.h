#pragma once
#include <memory>
#include "DamageChain.h"

namespace game::attack
{
	/**
	* @brief ダメージ計算チェーンのインターフェース(COR: Chain of Responsibility)
	*/
	class IDamageHander
	{
		virtual ~IDamageHander() = default;

		/**
		 * @brief 次のハンドラをセットする
		 * @param next 次のハンドラ
		 */
		virtual void setNext(std::unique_ptr<IDamageHander> next) = 0;

		/**
		 * @brief ダメージ計算を実行する
		 * @param next 攻撃計算コンテキスト
		 */
		virtual void handle(DamageChain& chain) = 0;
	};
}