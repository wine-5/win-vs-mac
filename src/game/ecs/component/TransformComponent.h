#pragma once
#include <DxLib.h>

namespace game::ecs::component
{
	/**
	 * @brief 位置・回転・大きさを持つコンポーネント
	 */
	struct TransformComponent
	{
		VECTOR m_position = { 0.0f, 0.0f, 0.0f };
		VECTOR m_rotation = { 0.0f, 0.0f, 0.0f };
		VECTOR m_scale    = { 1.0f, 1.0f, 1.0f };
	};
}