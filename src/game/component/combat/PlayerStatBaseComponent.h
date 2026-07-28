#pragma once

namespace game::component::combat
{
	/**
	 * @brief 強化を一切受けていない、素の能力値を保持する
	 *
	 * 「今この能力は強化されているか」を判定するための基準点。
	 * 現在値（AttackComponent・HealthComponent・PlayerStatsComponent）と突き合わせ、
	 * 上回っていればHUDが強化中として色を変える。
	 *
	 * 強化の出どころ（装備ファイル／Item／将来のバフ）を一切知らない作りにしてあるので、
	 * 新しい強化手段を足しても、現在値を書き換えさえすればHUDの表示は自動で追従する。
	 * ここへ書き込むのはプレイヤー生成時の一度きりで、以降は誰も変更しない
	 */
	struct PlayerStatBaseComponent
	{
		float m_maxHp{ 0.0f };
		float m_defence{ 0.0f };
		float m_attackPower{ 0.0f };
		float m_attackRange{ 0.0f };
		float m_criticalRate{ 0.0f };
		float m_moveSpeed{ 0.0f };
		float m_projectileSpeed{ 0.0f };
		float m_projectileRange{ 0.0f };
	};
} // namespace game::component::combat
