// src/game/ui/UIManager.h
#pragma once
#include "IUIElement.h"
#include "core/interface/IUIRenderer.h"
#include <vector>
#include <memory>

namespace game::ui
{
    /**
     * @brief UI要素を管理するクラス
     * 複数のUI要素を一括で更新・描画する
     */
    class UIManager
    {
    public:
        UIManager() = default;
        ~UIManager() = default;

        /**
         * @brief UI要素を追加
         * @param element 追加するUI要素（所有権を移譲）
         */
        void addElement(std::unique_ptr<IUIElement> element);

        /**
         * @brief すべてのUI要素を更新
         */
        void update();

        /**
         * @brief すべてのUI要素を描画
         * @param uiRenderer UI描画インターフェース
         */
        void draw(core::iface::IUIRenderer& uiRenderer) const;

    private:
        std::vector<std::unique_ptr<IUIElement>> m_elements;
    };
} // namespace game::ui