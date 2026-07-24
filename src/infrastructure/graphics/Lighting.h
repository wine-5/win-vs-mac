#pragma once
#include "core/interface/ILighting.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLibのライティングを操作するクラス
	 *
	 * 有効・無効、環境光、既定の平行光源を設定するだけの薄い層。
	 */
	class Lighting : public core::iface::ILighting
	{
	  public:
		/**
		 * @brief ライティング全体の有効・無効を切り替える
		 * @param enabled 有効にするならtrue
		 */
		void setEnabled(bool enabled) noexcept override;

		/**
		 * @brief 環境光を設定する
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 */
		void setAmbient(int r, int g, int b) noexcept override;

		/**
		 * @brief 既定の平行光源の向きと色を設定する
		 * @param direction 光の進む向き
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 */
		void setDirectionalLight(const core::Vector3& direction, int r, int g, int b) noexcept override;

		/**
		 * @brief 点光源を作成する
		 * @param position 光源のワールド座標
		 * @param range 光の届く距離
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 * @return 光源ハンドル（失敗時は-1）
		 */
		[[nodiscard]] int createPointLight(const core::Vector3& position, float range,
		    int r, int g, int b) override;

		/**
		 * @brief 点光源の位置を更新する
		 * @param lightHandle 光源ハンドル
		 * @param position 新しいワールド座標
		 */
		void setPointLightPosition(int lightHandle, const core::Vector3& position) noexcept override;

		/**
		 * @brief 点光源を破棄する
		 * @param lightHandle 光源ハンドル
		 */
		void destroyPointLight(int lightHandle) noexcept override;
	};
} // namespace infrastructure::graphics
