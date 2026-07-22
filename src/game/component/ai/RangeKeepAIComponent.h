#pragma once
#include "core/utility/Vector3.h"

namespace game::component::ai
{
	/**
	 * @brief 遠距離維持型AI用のコンポーネント
	 *
	 * プレイヤーから一定距離を保ちながら遠距離攻撃（弾発射等）をする敵用。
	 * 近づきすぎたら後退、遠ければ接近する。必要に応じてホバー（浮遊）する。
	 * RangeKeepAISystemが本コンポーネントの有無で「この敵は距離維持型」と判定する。
	 */
	struct RangeKeepAIComponent
	{
		float m_preferredDistanceMin{ 0.0f }; // この距離より近づかれたら後退する
		float m_preferredDistanceMax{ 0.0f }; // この距離より遠ければ接近する
		float m_hoverHeight{ 0.0f };          // 浮遊高度（0なら地上。Safariなど空を飛ぶ敵用）
		int m_strafeDirection{ 1 };           // ストレイフ（周回）の向き（+1:右回り / -1:左回り）。個体ごとに固定する
		float m_fireCooldown{ 0.0f };         // 遠距離攻撃（弾発射）の間隔（秒）。接触攻撃とは別系統
		float m_currentFireCooldown{ 0.0f };  // 現在の発射クールダウン残り（EnemyRangedAttackSystemが管理）
		float m_facingYawOffset{ 0.0f };      // 正面向きのyawオフセット（ラジアン）。機体モデルの正面軸のズレを補正する
		float m_attackAnimTimer{ 0.0f };      // 発射直後の演出（前のめり＋ポップ）の残り時間（秒）
		core::Vector3 m_baseScale{ 0.0f, 0.0f, 0.0f }; // 予備動作のスケール演出用に保持する基準スケール（0なら未取得）
		                                               // 索敵範囲外の徘徊は共通の PatrolComponent が担う（このコンポーネントには持たせない）
	};
} // namespace game::component::ai
