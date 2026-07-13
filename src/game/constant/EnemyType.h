#pragma once

namespace game::constant
{
	/**
	 * @brief 敵の種類
	 *
	 * ステージ配置データ（JSON）と生成処理の対応付けに使う
	 */
	enum class EnemyType
	{
		Xcode,  // 近接雑魚
		Safari, // 遠距離雑魚（飛行）
		Mac,    // ボス
	};
}
