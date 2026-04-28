#pragma once
namespace game::component
{
	/**
	 * @brief 体力を持つコンポーネント
	 */
	struct HealthComponent
	{
		float m_maxHp{ 100.0f };
		float m_currentHp{ 100.0f };
		float m_defence{ 5.0f };
		bool m_isDead{ false };
	};
}