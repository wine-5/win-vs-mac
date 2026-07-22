#pragma once
#include "core/data/MacMetadata.h"
#include "core/utility/Vector3.h"
#include <cstddef>

namespace game::component::ai
{
	/**
	 * @brief ボスAIの状態機械（FSM）の状態
	 */
	enum class MacState
	{
		Idle,            // プレイヤー未検知：待機
		Windup,          // 溜め：予兆を出しながら発動を待つ
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
	 * MacAISystemが本コンポーネントの有無で「この敵はボス」と判定し、
	 * FSMを駆動する。フェーズごとの挙動パラメータはmacData.jsonから読み込む。
	 */
	struct MacAIComponent
	{
		MacState m_state{ MacState::Idle };

		// 現在のフェーズ（型安全な識別子。添字ではない）
		core::data::MacPhase m_currentPhase{ core::data::MacPhase::Normal };

		// 次の技を抽選するまでの残り時間（秒）。Chase中に減っていく
		float m_actionTimer{ 0.0f };

		// 技アニメ・演出の再生ロック残り時間（秒）。>0の間は次の行動へ遷移しない
		float m_animLockTimer{ 0.0f };

		// --- 溜め（ウィンドアップ）：予兆を出してから技を発動するための状態 ---
		MacState m_pendingAction{ MacState::Chase };      // 溜め完了時に発動する技
		float m_windupTimer{ 0.0f };                      // 溜めの残り時間（秒）
		float m_windupDuration{ 0.0f };                   // 溜めの全体長（進行度計算用）
		core::Vector3 m_windupAimDir{ 0.0f, 0.0f, 0.0f }; // 溜め開始時に固定した狙い方向（予兆と発射で共有）

		// Phase2への移行を既に行ったか（1回だけ発火させるためのフラグ）
		bool m_phase2Triggered{ false };

		// macData.jsonから読み込んだフェーズ定義一式
		core::data::MacMetadata m_config{};

		/**
		 * @brief 現在フェーズのパラメータを取得する
		 * @return 現在フェーズのMacPhaseData
		 */
		[[nodiscard]] const core::data::MacPhaseData& currentPhase() const
		{
			return m_config.phase(m_currentPhase);
		}
	};
} // namespace game::component::ai
