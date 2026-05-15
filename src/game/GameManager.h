#pragma once
#include "game/data/FileEquipmentData.h"
#include "game/data/JobSelectionData.h"
#include "core/base/Singleton.h"

namespace game
{
    /**
     * @brief ゲーム全体の状態を管理するクラス
     * Singleton として唯一のインスタンスが存在し、シーン間で共有するデータを保持する
     */
    class GameManager : public core::base::Singleton<GameManager>
    {
    public:
        friend core::base::Singleton<GameManager>;

        /**
         * @brief FileEquipmentData への参照を返す
         * @return FileEquipmentData の参照
         */
        [[nodiscard]] data::FileEquipmentData& getFileEquipmentData() noexcept
        {
            return m_fileEquipmentData;
        }

        /**
         * @brief JobSelectionData への参照を返す
         * @return JobSelectionData の参照
         */
        [[nodiscard]] data::JobSelectionData& getJobSelectionData() noexcept
        {
            return m_jobSelectionData;
        }

        /**
         * @brief デフォルトコンストラクタ（Singleton初期化用）
         */
        GameManager() = default;

    private:

        data::FileEquipmentData m_fileEquipmentData{};
        data::JobSelectionData m_jobSelectionData{};
    };
}