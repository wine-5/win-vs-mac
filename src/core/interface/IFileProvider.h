#pragma once
#include <string>

namespace core::iface
{
    /**
     * @brief ファイル選択機能の抽象インターフェース
     * Platform層の実装（WindowsDataProvider）への依存を隠蔽する
     */
    class IFileProvider
    {
    public:
        virtual ~IFileProvider() = default;

        /**
         * @brief ファイル選択ダイアログを開き、選択されたファイルのフルパスを返す
         * @return 選択されたファイルのフルパス。キャンセル時は空文字列で返す
         */
        [[nodiscard]] virtual std::string selectFile() = 0;
    };
} // namespace core::iface