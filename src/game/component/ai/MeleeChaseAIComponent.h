#pragma once
#include "core/ecs/Entity.h"

namespace game::component::ai
{
	/**
	 * @brief 近接追跡型AI用のコンポーネント
	 *
	 * プレイヤーへ直進し、攻撃レンジに入ったら止まって近接攻撃する敵用。
	 * 共通データ（targetEntity・detectionRange・moveSpeed・currentAttackCooldown等）は
	 * AIComponentで管理し、本コンポーネントは振る舞い固有のパラメータは持たない。
	 * MeleeChaseAISystemが本コンポーネントの有無で「この敵は近接追跡型」と判定する。
	 */
	struct MeleeChaseAIComponent
	{
		// 近接追跡型は特別なパラメータを不要とする
		// （攻撃レンジはAttackComponent、移動速度はAIComponentが持つ）
	};
} // namespace game::component::ai
