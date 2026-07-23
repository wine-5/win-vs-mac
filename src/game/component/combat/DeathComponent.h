#pragma once

namespace game::component::combat
{
	/**
	 * @brief 死亡してから後始末されるまでの、死亡ライフサイクルの状態を持つコンポーネント
	 *
	 * 「死亡状態で後始末待ち」であることと、その進行状態だけを表す汎用データ。
	 * 演出の内容（敵の赤化＋ディゾルブ・落下バウンドなど）や時間の意味付けは
	 * 各Systemの責務とし、ここには演出固有の定数は持たせない（PlayerとEnemyで異なるため）
	 */
	struct DeathComponent
	{
		float m_elapsed{ 0.0f };      // 死亡してからの経過秒
		float m_fadeTimer{ 0.0f };    // 消失フェード開始からの経過秒
		bool m_fading{ false };       // 消失フェード中か
		bool m_hasLanded{ false };    // 地面に着地して静止したか（落下する敵用。バウンド完了で立つ。CollisionSystemが立てる）
		bool m_hasTouchedGround{ false }; // 一度でも地面に触れたか（落下死の揺れを止める判定用。初回接地で立つ。CollisionSystemが立てる）
		bool m_animFinished{ false };     // 死亡アニメが再生完了したか（アニメ持ちの敵用。EnemyDeathSystemが立てる）
	};
} // namespace game::component::combat
