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

		// 足元に床があり立っているか。GroundingSystemが毎フレーム更新し、
		// PhysicsSystemがジャンプの可否判定に使う（GroundingSystemはPhysicsSystemの後に走るため1フレーム遅れだが、接地判定には十分）
		bool m_isGrounded{ false };

		// 足元にある床の高さ（ワールドY）。GroundingSystemが毎フレーム更新する。
		// 浮遊敵が「地面から一定の高さ」を保つために使う（絶対高度だと階層ごとに浮き方が変わるため）
		float m_groundHeight{ 0.0f };

		// m_groundHeight が有効か。足元に床が1つも無ければ false（奈落の上など）
		bool m_hasGroundHeight{ false };
	};
} // namespace game::component::movement