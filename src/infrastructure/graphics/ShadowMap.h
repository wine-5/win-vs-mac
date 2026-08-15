#pragma once
#include "core/interface/IShadowMap.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLibのシャドウマップを操作するクラス
	 *
	 * ハンドルの生成・破棄と2パス描画の切り替えだけを担う薄い層。
	 * Lighting と対になり、平行光の「明暗」を Lighting が、「遮り」をこちらが受け持つ。
	 */
	class ShadowMap : public core::iface::IShadowMap
	{
	  public:
		ShadowMap() = default;

		/** @brief 生成済みのシャドウマップを破棄する */
		~ShadowMap() override;

		ShadowMap(const ShadowMap&) = delete;
		ShadowMap& operator=(const ShadowMap&) = delete;

		/**
		 * @brief シャドウマップを作成する
		 * @param resolution 一辺の解像度（2の冪であること）
		 * @return 作成できたらtrue
		 */
		[[nodiscard]] bool create(int resolution) override;

		/** @brief シャドウマップを破棄する */
		void destroy() noexcept override;

		/**
		 * @brief 使用可能な状態か
		 * @return 作成済みならtrue
		 */
		[[nodiscard]] bool isValid() const noexcept override;

		/**
		 * @brief 影を落とす平行光の向きを設定する
		 * @param direction 光の進む向き（正規化していなくてよい）
		 */
		void setLightDirection(const core::Vector3& direction) noexcept override;

		/**
		 * @brief シャドウマップが受け持つワールドの範囲を設定する
		 * @param center 範囲の中心
		 * @param halfSize 中心からの水平方向の広がり
		 * @param halfHeight 中心からの垂直方向の広がり
		 */
		void setDrawArea(const core::Vector3& center, float halfSize, float halfHeight) noexcept override;

		/**
		 * @brief 影の判定に加える補正深度を設定する
		 * @param depth 補正深度
		 */
		void setAdjustDepth(float depth) noexcept override;

		/** @brief 影を落とす物の描画を開始する */
		void beginCasterPass() noexcept override;

		/** @brief 影を落とす物の描画を終了する */
		void endCasterPass() noexcept override;

		/** @brief 影を受ける物の描画を開始する */
		void beginReceivePass() noexcept override;

		/** @brief 影を受ける物の描画を終了する */
		void endReceivePass() noexcept override;

	  private:
		int m_handle{ -1 };
	};
} // namespace infrastructure::graphics
