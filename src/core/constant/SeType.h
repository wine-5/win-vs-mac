#pragma once

namespace core::constant
{
    /**
     * @brief SE（効果音）の種類を表す列挙型
     */
    enum class SeType
    {
        None,

		// ---- プレイヤーの攻撃 ----
		AttackPlayer,    // プレイヤーの攻撃
		PlayerSwing1,    // 近接コンボ1段目の振り
		PlayerSwing2,    // 近接コンボ2段目の振り
		PlayerCharge,    // 溜め撃ちの溜め開始
		HitWindow,       // Window弾が命中した
		HitChargedWindow, // 溜め撃ちのWindow弾が命中した
		Critical,         // 会心の一撃が出た

		// ---- 被弾・撃破 ----
		HitEnemy,        // 敵がダメージを受けた
		HitPlayer,       // プレイヤーがダメージを受けた
        DeadEnemy,       // 敵が倒れた
		DeadPlayer,      // プレイヤーが倒れた

		// ---- プレイヤーの挙動 ----
		PlayerJump,     // ジャンプした
		PlayerFootstep, // 一歩ぶんの足音

		// ---- 敵 ----
		EnemyAlert, // 敵がプレイヤーを発見した
		EnemySlam,  // Xcodeの地面叩きつけ

		// ---- 進行・演出 ----
		BattleReady, // 開始演出のREADY
		BattleFight, // 開始演出のFIGHT!

		// ---- UI ----
		UiClick,      // ボタンなどを押した
		UiKeyPress,   // 選択の移動・キー入力
		UiFileSelect, // 装備するファイルを決定した

		// 今後追加したら AudioRepository の typeMap にも追加を忘れないように
    };
} // namespace core::constant
