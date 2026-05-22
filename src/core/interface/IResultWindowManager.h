#pragma once
#include "core/data/ResultData.h"

namespace core::iface
{
    /**
     * @brief リザルト画面のWindow管理のインターフェース
     */
    class IResultWindowManager
    {
    public:
        virtual ~IResultWindowManager() noexcept = default;

        /**
         * @brief ウィンドウを表示し、結果データを反映する
         * @param data リザルトデータ
         */
        virtual void show(const core::data::ResultData& data) noexcept = 0;

        /**
         * @brief メッセージポンプ（毎フレーム呼び出し）
         */
        virtual void pumpMessages() noexcept = 0;
    };
}
