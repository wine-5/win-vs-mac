#pragma once

namespace game::component::combat
{
	/**
	 * @brief プレイヤーの能力値のうち、他のコンポーネントに居場所が無いものを持つ
	 *
	 * 攻撃力・射程・会心率は AttackComponent、最大HP・防御力は HealthComponent が持つ。
	 * それらは敵とも共有する仕組みなのでここへは複製せず、二重管理を作らない。
	 * 逆に移動速度と弾の性能はSystemがコンストラクタ引数で抱え込んでいて外から動かせなかったため、
	 * この形で外へ出す。
	 *
	 * 書く側はプレイヤー生成時の初期化（PlayerData＋装備ボーナス）とItemの取得処理、
	 * 読む側は MoveSystem・PlayerRangedAttackSystem・左下HUD。
	 * 値を1か所に置くことで、Itemは「ここへ足す」だけで全員に効く
	 */
	struct PlayerStatsComponent
	{
		float m_moveSpeed{ 0.0f };       // 移動速度（ダッシュ倍率は掛ける前の値）
		float m_projectileSpeed{ 0.0f }; // Window弾の弾速
		float m_projectileRange{ 0.0f }; // Window弾の飛距離（弾速×寿命）
	};
} // namespace game::component::combat
