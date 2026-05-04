#pragma once
#include <string>

namespace core::iface
{
    /**
     * @brief ファイル選択処理の抽象インターフェース
     * Platform層の実装（WindowsFileSelector）への依存を隠蔽する
     */
    class IFileSelector
    {
    public:
        virtual ~IFileSelector() = default;

        /**
         * @brief ファイル選択ダイアログを開き、選択されたファイルのフルパスを返す
         * @return 選択されたファイルのフルパス。キャンセル時は空文字列で返す
         */
        [[nodiscard]] virtual std::string selectFile() = 0;
    };
}