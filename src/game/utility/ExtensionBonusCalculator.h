#pragma once
#include "game/data/FileExtensionType.h"
#include "game/data/FileExtensionBonus.h"

namespace game::utility
{
    /**
     * @brief FileExtensionType に応じたパラメータボーナスを算出するクラス
     */
    class ExtensionBonusCalculator
    {
    public:
        /**
         * @brief 拡張子種別に対応する FileExtensionBonus を返す
         * @param type ファイル拡張子グループ種別
         * @return 対応するボーナス値
         */
        [[nodiscard]] static constexpr data::FileExtensionBonus calculate(data::FileExtensionType type) noexcept
        {
            // 拡張子の種類に応じて指示付き初期化子で加算
            // TODO: 現在は値がハードコーディングになっているため今後はjsonもしくは無名名前空間などでマジックナンバーを削除する
            switch (type)
            {
            case data::FileExtensionType::Executable:
                return { .atk = 10.0f };
            case data::FileExtensionType::Document:
                return { .spd = 1.5f };
            case data::FileExtensionType::Image:
                return { .def = 8.0f };
            case data::FileExtensionType::Audio:
                return { .hp = 20.0f };
            case data::FileExtensionType::Archive:
                return { .atk = 3.0f, .spd = 0.5f, .def = 3.0f, .hp = 5.0f, .attackRange = 0.5f };
            case data::FileExtensionType::Unknown:
            default:
                return { .attackRange = 20.0f };
            }
        }
    };
}