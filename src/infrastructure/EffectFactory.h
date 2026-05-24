#pragma once
#include <unordered_map>
#include "core/interface/IEffectFactory.h"
#include "infrastructure/EffectPool.h"
#include "infrastructure/repository/EffectRepository.h"

namespace infrastructure
{
	/**
	 * @brief IEffectFactory の実装クラス
	 * EffectType ごとに EffectPool を持ち、エフェクトの再生・停止・更新を管理する
	 */
	class EffectFactory : public core::iface::IEffectFactory
	{
	public:
		/**
		* @brief エフェクトリソースを読み込み、各 EffectPool を初期化する
		*/
		void initialize() override;

		/**
		 * @brief エフェクトを再生する
		 * @param type エフェクトの種別
		 * @param position 再生位置
		 * @return エフェクトのプレイハンドル（失敗時は -1）
		 */
		int play(core::constant::EffectType type, core::Vector3 position) override;

		/**
		 * @brief エフェクトを強制停止する
		 * @param handle play() が返したハンドル
		 */
		void stop(int handle) override;

		/**
		 * @brief 再生終了したスロットを毎フレーム回収する
		 */
		void update() override;

	private:
		static constexpr int HIT_POOL_SIZE{ 10 }; // ヒットエフェクトの最大同時再生数

		EffectRepository m_repository{};
		std::unordered_map<core::constant::EffectType, EffectPool> m_pools{};

		// stop() でどのプールに返却するかを逆引きするためのマップ
		std::unordered_map<int, core::constant::EffectType> m_handleToType{};
	};
}