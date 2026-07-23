#pragma once

namespace core::constant
{
    /**
     * @brief SE（効果音）の種類を表す列挙型
     */
    enum class SeType
    {
        None,

        // ゲーム関連
        AttackWarrior,   // 戦士の攻撃
        AttackFire,      // 魔法使いの攻撃(いったん炎)
        AttackNinja,     // 忍者の攻撃（いったん手裏剣）
        HitEnemy,        // 敵がダメージを受けた（TODO: もしかしたらPlayerの職業ごとに音を変更する可能性あり）
        HitPlayer,       // プレイヤーがダメージを受けた
        DeadEnemy,       // 敵が倒れた

        // 今後追加したら AudioRepository の typeMap にも追加を忘れないように
    };
} // namespace core::constant