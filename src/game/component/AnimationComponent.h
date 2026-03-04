#pragma once

namespace game::component
{
    /**
     * @brief アニメーション情報を持つコンポーネント
     */
    template<typename TState>
    struct AnimationComponent
    {
        TState m_state;
        int m_animIndex = -1;
        float m_animTime = 0.0f;
        float m_animTotalTime = 0.0f;
    };
}