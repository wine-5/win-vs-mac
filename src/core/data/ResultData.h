#pragma once
#include <string>
#include <vector>
#include "core/data/Difficulty.h"

namespace core::data
{
    /**
     * @brief ゲーム結果データを保持する構造体
     */
    struct ResultData
    {
        /** @brief 勝利かどうか (true=全敵撃破, false=プレイヤー死亡) */
        bool m_isVictory{false};

		/** @brief プレイした難易度（ランクのタイム基準が難易度ごとに変わる） */
		Difficulty m_difficulty{ Difficulty::Normal };

		/** @brief 経過時間（秒） */
        float m_elapsedTime{0.0f};

        /** @brief 撃破した敵の数 */
        int m_killCount{0};

        /** @brief 被ダメージの合計 */
        float m_totalDamageTaken{0.0f};

        /** @brief 使用したファイルのパス一覧 */
        std::vector<std::string> m_usedFiles{};

		/**
		 * @brief 道中で拾った拡張子の種別名一覧（拾った順）
		 *
		 * 持ち込んだファイル（m_usedFiles）とは分けて持つ。
		 * 出どころが違うものを混ぜると、リザルトで「何を持ち込んで何を拾ったか」を
		 * 分けて見せられなくなる
		 */
		std::vector<std::string> m_acquiredExtensions{};

		/** @brief そのうち能力に乗っていた個数（先頭から数えた装備中のぶん） */
		int m_equippedExtensionCount{ 0 };
	};
} // namespace core::data
