#pragma once
#include "core/Vector3.h"
#include "core/interface/ICamera.h"

namespace infrastructure
{
	/**
	 * @brief アイソメトリックカメラの制御クラス
	 */
	class Camera : public core::iface::ICamera
	{
	public:
		void update(const core::Vector3& targetPosition, const core::Vector3& offset) override;
	};
}