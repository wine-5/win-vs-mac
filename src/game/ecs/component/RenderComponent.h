#pragma once
#include <DxLib.h>

namespace game::ecs::component
{
    /**
     * @brief 描画情報を持つコンポーネント
     */
    struct RenderComponent
    {
        int m_modelHandle = -1; // -1は未ロード
        bool m_isVisible = true;
    };
}