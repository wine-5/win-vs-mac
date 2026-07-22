#pragma once
#include "core/utility/Vector3.h"

namespace game::component::ai
{
	/**
	 * @brief 近接追跡型AIの行動状態
	 */
	enum class MeleeChaseState
	{
		Patrol, // 索敵範囲外：スポーン地点まわりを徘徊する
		Chase,  // 索敵範囲内：プレイヤーへ接近し、レンジ内で攻撃する
	};

	/**
	 * @brief 近接追跡型AI用のコンポーネント
	 *
	 * プレイヤーが索敵範囲内にいれば接近して近接攻撃し、範囲外では
	 * スポーン地点まわりを徘徊する。共通データ（targetEntity・detectionRange・
	 * moveSpeed・currentAttackCooldown等）はAIComponentが持ち、徘徊の基準点や目的地は
	 * 共通の PatrolComponent が持つ。本コンポーネントは近接追跡型に固有の行動ステートだけを持つ。
	 * MeleeChaseAISystemが本コンポーネントの有無で「この敵は近接追跡型」と判定する。
	 */
	struct MeleeChaseAIComponent
	{
		MeleeChaseState m_state{ MeleeChaseState::Patrol };
	};
} // namespace game::component::ai
