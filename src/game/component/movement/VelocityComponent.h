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

		// 本人の意思とは別に外から加わる速度（坂を滑り落ちる等）。
		// MoveSystemが m_velocity を毎フレーム上書きするため、混ぜずに別枠で持ち、
		// PhysicsSystemが移動時に合算する。歩き速度との大小がそのまま挙動になる
		core::Vector3 m_externalVelocity{};
	};
} // namespace game::component::movement