#pragma once
#include <memory>
#include <random>
#include "IDamageHandler.h"
#include "core/ecs/ComponentManager.h"

namespace game::attack
{
	/**
	 * @brief 一定確率でダメージを倍化するハンドラ（クリティカル）
	 *
	 * 攻撃者の AttackComponent が持つ発生率で抽選し、当たればダメージへ倍率を掛ける。
	 * 発生率の既定は0なので、値を設定していないEntity（敵など）では一度も発生しない。
	 *
	 * 防御力の減算より後ろに置くこと。先に倍化してから防御力を引くと、
	 * 防御の高い相手ほどクリティカルの旨味が消えて「効いた感じ」がしなくなる
	 */
	class CriticalHandler : public IDamageHandler
	{
	  public:
		/**
		 * @brief CriticalHandlerのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		explicit CriticalHandler(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 次のハンドラをセットする
		 * @param next 次のハンドラ
		 */
		void setNext(std::unique_ptr<IDamageHandler> next) override;

		/**
		 * @brief クリティカルを抽選し、当たればダメージを倍化して次のハンドラに渡す
		 * @param chain 攻撃計算コンテキスト
		 */
		void handle(DamageChain& chain) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		std::unique_ptr<IDamageHandler> m_next{};

		// 起動のたびに出目を変える。他のSystemと同じくmt19937を使う
		// （std::randはこのプロジェクトのどこでもsrandしておらず、毎回同じ並びになる）
		std::mt19937 m_rng{ std::random_device{}() };
		std::uniform_real_distribution<float> m_distribution{ 0.0f, 1.0f };
	};
} // namespace game::attack
