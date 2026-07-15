#pragma once
#include "core/interface/IUIRenderer.h"

namespace game::ui
{
    /**
     * @brief UI要素の状態
     */
    enum class UIState
    {
        Normal,   // 通常
        Focused,  // フォーカス中
        Pressed,  // 押下中
        Disabled  // 無効
    };

    /**
     * @brief UI要素の基底インターフェース
     */
    class IUIElement
    {
    public:
        virtual ~IUIElement() = default;

        /**
         * @brief UI要素を更新する
         */
        virtual void update() = 0;

        /**
         * @brief UI要素を描画する
         * @param uiRenderer UI描画インターフェース
         */
        virtual void draw(core::iface::IUIRenderer& uiRenderer) const = 0;

        /**
         * @brief 表示状態を設定する
         * @param visible 表示するかどうか
         */
        virtual void setVisible(bool visible) = 0;
    };
} // namespace game::ui