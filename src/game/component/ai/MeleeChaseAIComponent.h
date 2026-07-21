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
	 * moveSpeed・currentAttackCooldown等）はAIComponentが持ち、本コンポーネントは
	 * 近接追跡型に固有の状態（行動ステート・徘徊の基準点や目的地）を持つ。
	 * MeleeChaseAISystemが本コンポーネントの有無で「この敵は近接追跡型」と判定する。
	 */
	struct MeleeChaseAIComponent
	{
		MeleeChaseState m_state{ MeleeChaseState::Patrol };

		// 徘徊の基準点（スポーン地点）。初回更新時に現在位置で初期化する
		core::Vector3 m_homePosition{};
		bool m_homeInitialized{ false };

		// 現在の徘徊目的地
		core::Vector3 m_wanderTarget{};
		bool m_hasWanderTarget{ false };

		// 目的地到着後にその場で立ち止まる残り時間（秒）
		float m_pauseTimer{ 0.0f };
	};
} // namespace game::component::ai
