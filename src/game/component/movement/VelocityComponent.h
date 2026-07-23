#pragma once
#include "core/utility/Vector3.h"

namespace game::component::movement
{
	/**
	 * @brief 速度を持つコンポーネント
	 */
	struct VelocityComponent
	{
		core::Vector3 m_velocity;
	};
} // namespace game::component::movement