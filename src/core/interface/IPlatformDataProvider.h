#pragma once
#include <string>

namespace core::iface
{
    /**
     * @brief プラットフォーム由来のデータを提供する抽象インターフェース
     * Platform層の実装（WindowsDataProvider）への依存を隠蔽する
     */
    class IPlatformDataProvider
    {
    public:
        virtual ~IPlatformDataProvider() = default;

        /**
         * @brief ファイル選択ダイアログを開き、選択されたファイルのフルパスを返す
         * @return 選択されたファイルのフルパス。キャンセル時は空文字列で返す
         */
        [[nodiscard]] virtual std::string selectFile() = 0;
    };
}