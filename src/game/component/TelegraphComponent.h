#pragma once
#include "core/utility/Vector3.h"

namespace game::component
{
	/**
	 * @brief 攻撃予兆（テレグラフ）の形状
	 */
	enum class TelegraphShape
	{
		Circle, // 円（近接・着弾範囲）
		Sector, // 扇形（扇状攻撃の範囲）
	};

	/**
	 * @brief 攻撃予兆（テレグラフ）の表示状態を持つコンポーネント
	 *
	 * ボス等がこれを持ち、攻撃の溜め（ウィンドアップ）中に危険範囲を地面へ予告表示する。
	 * 形状・範囲・進行度だけを保持し、実際の描画は TelegraphVisualsSystem が担う
	 * （AI/攻撃ロジックと描画を分離する）。AttackComponentのワインドアップに紐づく
	 * 近接専用の予兆（AttackTelegraphVisualsSystem）とは別系統で、扇など任意形状を出せる。
	 */
	struct TelegraphComponent
	{
		bool m_active{ false };                           // 予兆を表示中か
		TelegraphShape m_shape{ TelegraphShape::Circle }; // 予兆の形状
		core::Vector3 m_center{ 0.0f, 0.0f, 0.0f };       // 予兆の中心（ワールド座標。通常は攻撃者の足元）
		float m_facingRad{ 0.0f };                        // 扇の中心方向（Sector用。ラジアン、atan2(dz,dx)）
		float m_radius{ 0.0f };                           // 半径（ワールド単位）
		float m_halfAngleRad{ 0.0f };                     // 扇の片側の開き角（Sector用。全開き角の半分）
		float m_progress{ 0.0f };                         // 危険の切迫度（0→1。満ちきると発動）
	};
} // namespace game::component
