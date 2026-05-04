#pragma once
#include "core/ecs/Entity.h"

namespace game::component
{
	/**
     * @brief AI追従行動を制御するコンポーネント	
     */
	struct AIComponent
	{
		core::ecs::Entity m_targetEntity{ 0 };
		// 以下はJSON未設定時に即座に異常検知できるようにするための意図的な初期値
		float m_moveSpeed{ 0.0f };
		float m_detectionRange{ 0.0f };
		float m_attackCooldown{ 0.0f }; // TODO: AIの行動パターンによっては削除する可能性あり
		float m_currentAttackCooldown{};
		bool m_isActive{ true };
	};
}
