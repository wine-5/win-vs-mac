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
		float m_moveSpeed{ 2.0f };
		float m_detectionRange{ 10.0f };
		bool m_isActive{ true };
	};
}
