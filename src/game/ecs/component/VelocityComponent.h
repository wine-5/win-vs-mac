#pragma once
#include <DxLib.h>

namespace game::ecs::component
{
	/**
	 * @brief 速度を持つコンポーネント
	 */
	struct VelocityComponent
	{
		VECTOR m_velocity = { 0.0f, 0.0f, 0.0f };
	};
}