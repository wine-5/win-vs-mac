#pragma once

namespace game::component
{
	/**
	 * @brief 死亡後、Entity破棄までの残り時間を管理する汎用コンポーネント
	 *
	 * 「死亡状態で後始末待ち」であることと、その残り時間だけを表す。
	 * 演出の内容（敵の赤化＋ディゾルブなど）や合計時間は各Systemが持つ責務とし、
	 * ここには持たせない（PlayerとEnemyで演出が異なるため）
	 */
	struct DeathComponent
	{
		float m_timer{ 0.0f };
	};
} // namespace game::component
