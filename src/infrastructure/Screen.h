// src/infrastructure/Screen.h
#pragma once
#include "core/interface/IScreen.h"

namespace infrastructure
{
    /**
     * @brief 画面情報を提供するクラス
     * DxLibから画面サイズを取得する
     */
    class Screen : public core::iface::IScreen
    {
    public:
        /**
         * @brief DxLibから画面サイズを取得して初期化
         */
        Screen();

        /**
         * @brief 画面幅を取得
         * @return 画面の幅（ピクセル）
         */
        int getWidth() const noexcept override;

        /**
         * @brief 画面高さを取得
         * @return 画面の高さ（ピクセル）
         */
        int getHeight() const noexcept override;

    private:
        int m_width;
        int m_height;
    };
}