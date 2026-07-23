#pragma once
#include "core/utility/Vector3.h"

namespace game::component::movement
{
	/**
	 * @brief 位置・回転・大きさを持つコンポーネント
	 */
	struct TransformComponent
	{
		core::Vector3 m_position;
		core::Vector3 m_rotation;
		core::Vector3 m_scale{1.0f, 1.0f, 1.0f};
	};
} // namespace game::component::movement