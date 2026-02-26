#pragma once
#include <DxLib.h>

namespace game::ecs::component
{	/**
	 * @brief カメラの情報を持つコンポーネント
	 */
	struct CameraComponent
	{
		static constexpr float OFFSET_Y = 800.0f;
		static constexpr float OFFSET_Z = -600.0f;

		VECTOR m_offset = { 0.0f, OFFSET_Y, OFFSET_Z }; // Playerからカメラまでの相対位置
		VECTOR m_target = { 0.0f,0.0f, 0.0f };
		VECTOR m_position = { 0.0f, 0.0f, 0.0f };
	};
}