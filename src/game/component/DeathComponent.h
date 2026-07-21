#pragma once

namespace game::component
{
	/**
	 * @brief 死亡後、Entity破棄までの演出待機時間を管理するコンポーネント
	 *
	 * EnemyDeathSystemがEnemyDeadEvent受信時に付与し、タイマーが0になったら
	 * Entity・Componentを破棄しモデルハンドルをプールへ返却する
	 */
	struct DeathComponent
	{
		float m_timer{ 1.0f };
	};
} // namespace game::component
