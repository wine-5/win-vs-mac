#pragma once

namespace game::component
{
	/**
	 * @brief プレイヤーの入力状態を持つコンポーネント
	 */
	struct InputComponent
	{
		float m_moveX{ 0.0f };
		float m_moveZ{ 0.0f };
		bool  m_jumpPressed{ false };
		bool  m_attackPressed{ false };
		bool m_dashPressed{ false };         // Shift押下でダッシュ
		bool m_rangedAttackPressed{ false }; // 右クリックで遠距離攻撃
	};
} // namespace game::component