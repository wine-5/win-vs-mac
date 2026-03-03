#pragma once
#include "core/Vector3.h"

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
		virtual void update(const core::Vector3& targetPosition, const core::Vector3& offset) = 0;
	};
}