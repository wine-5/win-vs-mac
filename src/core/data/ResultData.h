#pragma once
#include <string>
#include <vector>

namespace core::data
{
    /**
     * @brief ゲーム結果データを保持する構造体
     */
    struct ResultData
    {
        /** @brief 勝利かどうか (true=全敵撃破, false=プレイヤー死亡) */
        bool m_isVictory{false};

        /** @brief 経過時間（秒） */
        float m_elapsedTime{0.0f};

        /** @brief 撃破した敵の数 */
        int m_killCount{0};

        /** @brief 被ダメージの合計 */
        float m_totalDamageTaken{0.0f};

        /** @brief 使用したファイルのパス一覧 */
        std::vector<std::string> m_usedFiles{};
    };
}
