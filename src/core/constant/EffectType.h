#pragma once

namespace core::constant
{
	/**
	 * @brief エフェクトの種類を表す列挙型
	 */
	enum class EffectType
	{
		None,
		Enemy_HitSword,  // 敵がプレイヤーの剣（近接）で被弹
		Enemy_HitWindow, // 敵がプレイヤーのWindow投撃弾（遠距離）で被弾
		Enemy_Spawn,     // 敵スポーン（テスト用：Tキーでプレイヤー位置に再生）
		Player_Slash,    // プレイヤーが左クリックで剣攻撃したときの斬撃エフェクト
		Enemy_Slash,     // 敵が攻撃（近接/遠距離問わず攻撃開始時）を行ったときのエフェクト

		// 今後 Player専用エフェクトを追加する場合は Player_で統一する（例: Player_HitSword）
		// EffectRepositoryのtypeMapにも追加を忘れないように
	};
} // namespace core::constant