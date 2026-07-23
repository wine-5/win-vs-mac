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
		~EffectFactory();

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

		/**
		 * @brief エフェクトが再生中かどうかを返す
		 * @param handle play() が返したハンドル
		 * @return 再生中なら true
		 */
		bool isPlaying(int handle) const override;

		/**
		 * @brief 再生中のエフェクトをすべて描画する
		 */
		void draw() override;

		// DEBUG: 負荷の原因特定用（リリース時に削除）
		[[nodiscard]] int getActiveEffectCount() const noexcept override;

	  private:
		repository::EffectRepository m_repository{};
		std::unordered_map<core::constant::EffectType, EffectPool> m_pools{};

		// stop() でどのプールに返却するかを逆引きするためのマップ
		std::unordered_map<int, core::constant::EffectType> m_handleToType{};
	};
} // namespace infrastructure