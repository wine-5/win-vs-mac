#pragma once
#include <memory>
#include "DamageChain.h"

namespace game::attack
{
	/**
	* @brief ダメージ計算チェーンのインターフェース(COR: Chain of Responsibility)
	*/
	class IDamageHandler
	{
	public:
		virtual ~IDamageHandler() = default;

		/**
		 * @brief 次のハンドラをセットする
		 * @param next 次のハンドラ
		 */
		virtual void setNext(std::unique_ptr<IDamageHandler> next) = 0;

		/**
		 * @brief ダメージ計算を実行する
		 * @param chain 攻撃計算コンテキスト
		 */
		virtual void handle(DamageChain& chain) = 0;
	};
}