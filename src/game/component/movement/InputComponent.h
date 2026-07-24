#pragma once

namespace game::component::movement
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

		// trueの間は全入力を無効化する（ボス覚醒などのシネマ演出中）。演出Systemが書く
		bool m_locked{ false };
	};
} // namespace game::component::movement