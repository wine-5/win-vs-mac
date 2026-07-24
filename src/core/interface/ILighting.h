#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 3Dライティングを操作するインターフェース
	 *
	 * Game層がDxLibへ直接触らずに、明暗の演出（虚無の暗さと光の当たり方）を制御するための抽象化。
	 */
	class ILighting
	{
	  public:
		virtual ~ILighting() = default;

		/**
		 * @brief ライティング全体の有効・無効を切り替える
		 * @param enabled 有効にするならtrue
		 */
		virtual void setEnabled(bool enabled) noexcept = 0;

		/**
		 * @brief 光が直接当たらない面の明るさ（環境光）を設定する
		 *
		 * 虚無の世界観では低めにして陰影を残しつつ、テクスチャの模様が
		 * 潰れない程度の下限を確保する。
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 */
		virtual void setAmbient(int r, int g, int b) noexcept = 0;

		/**
		 * @brief 既定の平行光源の向きと色を設定する
		 * @param direction 光の進む向き（正規化していなくてよい）
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 */
		virtual void setDirectionalLight(const core::Vector3& direction, int r, int g, int b) noexcept = 0;
	};
} // namespace core::iface
