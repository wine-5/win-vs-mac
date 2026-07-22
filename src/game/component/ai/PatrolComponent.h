#pragma once
#include "core/utility/Vector3.h"

namespace game::component::ai
{
	/**
	 * @brief 索敵範囲外にプレイヤーがいる間、ホーム（スポーン地点）周辺をふらつく徘徊の状態
	 *
	 * 近接追跡型（MeleeChaseAISystem）・距離維持型（RangeKeepAISystem）など、敵の種類に
	 * 依らず「発見していない間の徘徊」を共通データとして持たせるためのコンポーネント。
	 * 具体的な移動のさせ方（地上を歩く／浮遊して移動する）は各AI Systemの責務とし、
	 * ここには徘徊の基準点・目的地・待機タイマーだけを持たせる。
	 */
	struct PatrolComponent
	{
		core::Vector3 m_homePosition{ 0.0f, 0.0f, 0.0f }; // 徘徊の基準点（スポーン地点）。初回更新時に現在位置で初期化する
		bool m_homeInitialized{ false };                  // m_homePositionを初期化済みか
		core::Vector3 m_wanderTarget{ 0.0f, 0.0f, 0.0f }; // 現在の徘徊目的地（水平のみ使う）
		bool m_hasWanderTarget{ false };                  // 徘徊目的地を保持中か
		float m_pauseTimer{ 0.0f };                       // 目的地到着後にその場で待機する残り時間（秒）
	};
} // namespace game::component::ai
