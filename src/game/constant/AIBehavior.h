#pragma once

namespace game::constant
{
	/**
	 * @brief 敵AIの行動タイプ
	 *
	 * 行動ロジック自体はAISystemが本タイプで分岐して実装する。
	 * 敵クラス（XcodeEnemy等）はスポーン時にこの値を設定するだけでよい
	 */
	enum class AIBehavior
	{
		MeleeChase,       // 近距離追跡型：プレイヤーへ直進し攻撃範囲で近接攻撃
		RangeKeepDistance,// 遠距離維持型：一定距離を保ちながら遠距離攻撃
		Boss              // ボス型：接近、遠距離などを使い分ける
	};
}