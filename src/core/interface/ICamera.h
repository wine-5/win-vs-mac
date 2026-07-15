#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief カメラ装置の純粋仮想クラス
	 * Game層がInfrastructure層に直接依存しないための抽象化。
	 * yaw/pitch等の計算は行わず、渡された値をそのまま設定するだけの薄い層。
	 */
	class ICamera
	{
	public:
		virtual ~ICamera() = default;

		/**
		 * @brief カメラの位置と注視点を設定する
		 * @param position カメラのワールド座標
		 * @param target 注視点のワールド座標
		 */
		virtual void setLookAt(const core::Vector3& position, const core::Vector3& target) = 0;

		/**
		 * @brief 視野角（FOV）を設定する（将来のズーム演出用）
		 * @param fovRad 視野角（ラジアン）
		 */
		virtual void setFieldOfView(float fovRad) = 0;
	};
} // namespace core::iface
