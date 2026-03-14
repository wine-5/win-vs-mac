// src/infrastructure/Screen.h
#pragma once
#include "core/interface/IScreen.h"

namespace infrastructure
{
    /**
     * @brief 画面情報を提供するクラス
     * SetGraphMode()で設定した画面サイズを保持する
     */
    class Screen : public core::iface::IScreen
    {
    public:
        /**
         * @brief 画面サイズを指定して初期化
         * @param width 画面幅
         * @param height 画面高さ
         */
        Screen(int width, int height);

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