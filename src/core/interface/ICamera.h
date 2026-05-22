#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief カメラの純粋仮想クラス
	 * Game層がInfrastructre層に直接依存しないための抽象化
	 */
	class ICamera
	{
	public:
		virtual ~ICamera() = default;
		
		/**
		 * @brief カメラの位置をターゲットに追従させる
		 * @param targetPosition 追従対象の座標
		 * @param offset ターゲットからのオフセット
		 */
		virtual void update(const core::Vector3& targetPosition, const core::Vector3& offset) = 0;
	};
}