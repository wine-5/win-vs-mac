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
	};
}