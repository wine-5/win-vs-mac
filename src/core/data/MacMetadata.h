#pragma once
#include <string>
#include <vector>

namespace core::data
{
	/**
	 * @brief ボスのフェーズ識別子
	 *
	 * 「今どのフェーズか」を表す型安全な識別子。
	 */
	enum class MacPhase
	{
		Normal,  // 初期フェーズ
		Awakened // 覚醒フェーズ（HP比率が閾値を下回ると移行）
	};

	/**
	 * @brief ボスの1フェーズ分の挙動パラメータ
	 *
	 * macData.json の "mac.phase1" / "mac.phase2" に対応する。
	 * フェーズ切替（覚醒）で参照するフェーズを丸ごと差し替える。
	 */
	struct MacPhaseData
	{
		float m_moveSpeed{ 0.0f };      // このフェーズでの移動速度
		float m_actionInterval{ 0.0f }; // 次の技を抽選するまでの間隔（秒）
		float m_meleeRange{ 0.0f };     // 近接技を候補に入れる距離

		// 技の重み（重み付きランダム抽選に使う。0なら候補から外れる）
		int m_weightMelee{ 0 };  // 接近攻撃
		int m_weightRanged{ 0 }; // 遠方から攻撃
		int m_weightSummon{ 0 }; // 敵の生成
		int m_weightNova{ 0 };   // 全方位レインボー・ノヴァ（覚醒限定。phase1は0で候補外）

		// 遠距離（レインボー扇状）
		int m_rainbowCount{ 0 };          // 1回の発射数
		float m_rainbowSpreadDeg{ 0.0f }; // 扇の全開き角（度）
		float m_rainbowSpeed{ 0.0f };     // 弾速（0ならprojectileData.jsonの既定値を使う）
		float m_rainbowSpinSpeed{ 0.0f }; // ルーレット回転の速さ（1ワールド単位進むごとの回転量[rad]）

		// 全方位ノヴァ（覚醒限定）
		int m_novaCount{ 0 }; // 360度に均等発射するレインボー弾の数

		// 召喚
		std::vector<std::string> m_summonTypes{}; // 召喚候補の敵タイプ名（"xcode" / "safari"）
		int m_summonCount{ 0 };                   // 1回の召喚数
		int m_summonMax{ 0 };                     // 同時存在できる召喚上限
		float m_summonRadiusMin{ 0.0f };          // ボス周辺の召喚半径（最小）
		float m_summonRadiusMax{ 0.0f };          // ボス周辺の召喚半径（最大）
	};

	/**
	 * @brief ボス全体の挙動定義
	 *
	 * macData.json の "mac" オブジェクトに対応する。
	 * フェーズは名前付きフィールドで保持し、HP比率で Phase1 → Phase2 に切り替える。
	 * 添字ではなく MacPhase 列挙型で参照するため、範囲外アクセスの危険がない。
	 */
	struct MacMetadata
	{
		float m_phase2HpRatio{ 0.5f }; // このHP比率を下回ると Phase2 に移行する
		bool m_hasPhase2{ false };     // Phase2が定義されているか（未定義ならフェーズ移行しない）

		MacPhaseData m_phase1{}; // 初期フェーズのパラメータ
		MacPhaseData m_phase2{}; // 覚醒フェーズのパラメータ

		/**
		 * @brief 指定フェーズのパラメータを取得する
		 * @param phase フェーズ識別子
		 * @return 対応するフェーズのMacPhaseData
		 */
		[[nodiscard]] const MacPhaseData& phase(MacPhase phase) const noexcept
		{
			return (phase == MacPhase::Awakened) ? m_phase2 : m_phase1;
		}
	};
} // namespace core::data
