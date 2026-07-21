#pragma once
#include "core/ecs/Entity.h"

namespace game::component
{
	/**
	 * @brief AI追従行動を制御するコンポーネント
	 *
	 * 行動タイプの判別は各AI専用マーカーコンポーネント
	 * （MeleeChaseAIComponent / RangeKeepAIComponent / BossAIComponent）の有無で行う
	 */
	struct AIComponent
	{
		core::ecs::Entity m_targetEntity{ 0 };

		// 以下はJSON未設定時に即座に異常検知できるようにするための意図的な初期値
		float m_moveSpeed{ 0.0f };
		float m_detectionRange{ 0.0f };
		float m_attackCooldown{ 0.0f }; // TODO: AIの行動パターンによっては削除する可能性あり
		float m_currentAttackCooldown{};
		bool m_isActive{ true };

		// 前フレームでプレイヤーを索敵範囲内に捉えていたか。
		// DetectionSystemが「未索敵→索敵」の切り替わり（発見の瞬間）を検知するのに使う
		bool m_wasAware{ false };

		// 距離維持型用のパラメータ
		float m_preferredDistanceMin{0.0f}; // これより近づかれたら後退する
		float m_preferredDistanceMax{0.0f}; // これより遠ければ接近する
		float m_hoverHeight{0.0f}; // 浮遊高度(0なら地上)
	};
} // namespace game::component
