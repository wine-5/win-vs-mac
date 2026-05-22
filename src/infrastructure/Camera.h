#pragma once
#include "core/utility/Vector3.h"
#include "core/interface/ICamera.h"

namespace infrastructure
{
	/**
	 * @brief アイソメトリックカメラの制御クラス
	 */
	class Camera : public core::iface::ICamera
	{
	public:
		/**
		 * @brief カメラの位置をターゲットに追従させる
		 * @param targetPosition 追従対象の座標
		 * @param offset ターゲットからのオフセット
		 */
		void update(const core::Vector3& targetPosition, const core::Vector3& offset) override;
	};
}