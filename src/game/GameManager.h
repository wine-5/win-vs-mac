#pragma once
#include "game/data/FileEquipmentData.h"

namespace game
{
    /**
     * @brief ゲーム全体の状態を管理するクラス
     * ServiceLocator に登録され、シーン間で共有するデータを保持する
     */
    class GameManager
    {
    public:
        /**
         * @brief FileEquipmentData への参照を返す
         * @return FileEquipmentData の参照
         */
        [[nodiscard]] data::FileEquipmentData& getFileEquipmentData() noexcept
        {
            return m_fileEquipmentData;
        }

    private:
        data::FileEquipmentData m_fileEquipmentData{};
    };
}