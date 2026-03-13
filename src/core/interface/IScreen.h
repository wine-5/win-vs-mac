#pragma once

namespace core::iface
{   
    /**
     * @brief 画面サイズを提供するインターフェース
     */
    class IScreen
    {
    public:
        virtual ~IScreen() = default;

        /**
         * @brief 画面幅を取得
         * @return 画面の幅（ピクセル）
         */
        virtual int getWidth() const noexcept = 0;

        /**
         * @brief 画面高さを取得
         * @return 画面の高さ（ピクセル）
         */
        virtual int getHeight() const noexcept = 0;
    };
}