#pragma once

namespace game::component
{
	/**
	 * @brief 攻撃を持つコンポーネント
	 */
	struct AttackComponent
	{
		float m_attackPower{ 10.0f };
		float m_attackRange{ 100.0f };
		float m_attackCooldown{ 2.0f };
		float m_currentCooldown{ 0.0f };
		bool  m_attackRequested{ false };
	};
}