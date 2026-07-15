#pragma once
namespace game::component
{
	/**
	 * @brief 体力を持つコンポーネント
	 */
	struct HealthComponent
	{
		// 0はJSON未設定時に即座に異常検知できるようにするための意図的な初期値
		float m_maxHp{ 0.0f };
		float m_currentHp{ 0.0f };
		float m_defence{ 0.0f };
		bool  m_isDead{ false };
	};
} // namespace game::component