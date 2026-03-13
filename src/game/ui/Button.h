#pragma once
#include "IUIElement.h"
#include "core/interface/IInputProvider.h"
#include <string>
#include <functional>

namespace game::ui
{
    /**
     * @brief ボタンのUI要素
     */
    class Button : public IUIElement
    {
    public:
        /**
         * @brief Buttonのコンストラクタ
         * @param text ボタンのテキスト
         * @param x X座標
         * @param y Y座標
         * @param width 幅
         * @param height 高さ
         * @param inputProvider 入力インターフェース
         */
        Button(const std::string& text, int x, int y, int width, int height,
            core::iface::IInputProvider& inputProvider);

        void update() override;
        void draw(core::iface::IUIRenderer& uiRenderer) const override;
        void setVisible(bool visible) override;

        /**
         * @brief クリック時のコールバックを設定する
         * @param callback コールバック関数
         */
        void setOnClick(std::function<void()> callback);

    private:
        /**
         * @brief マウスがボタン上にあるか判定する
         * @return マウスがボタン上にある場合true
         */
        bool isMouseOver() const;

        /**
         * @brief 状態に応じた色を取得する
         * @return 色（ARGB）
         */
        unsigned int getColor() const;

        std::string m_text;
        int m_x{};
        int m_y{};
        int m_width{};
        int m_height{};
        UIState m_state = UIState::Normal;
        bool m_visible{};
        bool m_wasPressed{};
        std::function<void()> m_onClick{};
        core::iface::IInputProvider& m_inputProvider;
    };
}