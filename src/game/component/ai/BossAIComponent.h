#pragma once
#include "core/data/BossMetadata.h"
#include <cstddef>

namespace game::component::ai
{
	/**
	 * @brief ボスAIの状態機械（FSM）の状態
	 */
	enum class BossState
	{
		Idle,            // プレイヤー未検知：待機
		Chase,           // プレイヤーへ接近
		Melee,           // 近接攻撃（アニメ再生中はロック）
		Ranged,          // 遠距離攻撃（レインボー扇状）
		Summon,          // 雑魚召喚
		PhaseTransition, // フェーズ移行（覚醒演出中）
		Dead             // 死亡
	};

	/**
	 * @brief ボス型AI用のコンポーネント
	 *
	 * 近接・遠距離・召喚を使い分け、HP比率でフェーズ遷移するボス敵用。
	 * BossAISystemが本コンポーネントの有無で「この敵はボス」と判定し、
	 * FSMを駆動する。フェーズごとの挙動パラメータはmacData.jsonから読み込む。
	 */
	struct BossAIComponent
	{
		BossState m_state{ BossState::Idle };

		// 現在のフェーズ（型安全な識別子。添字ではない）
		core::data::BossPhase m_currentPhase{ core::data::BossPhase::Normal };

		// 次の技を抽選するまでの残り時間（秒）。Chase中に減っていく
		float m_actionTimer{ 0.0f };

		// 技アニメ・演出の再生ロック残り時間（秒）。>0の間は次の行動へ遷移しない
		float m_animLockTimer{ 0.0f };

		// Phase2への移行を既に行ったか（1回だけ発火させるためのフラグ）
		bool m_phase2Triggered{ false };

		// macData.jsonから読み込んだフェーズ定義一式
		core::data::BossMetadata m_config{};

		/**
		 * @brief 現在フェーズのパラメータを取得する
		 * @return 現在フェーズのBossPhaseData
		 */
		[[nodiscard]] const core::data::BossPhaseData& currentPhase() const
		{
			return m_config.phase(m_currentPhase);
		}
	};
} // namespace game::component::ai
