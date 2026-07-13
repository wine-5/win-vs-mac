#pragma once
#include <unordered_map>
#include "game/component/AnimationClip.h"
#include "game/constant/AnimationState.h"

namespace game::component
{
    /**
     * @brief アニメーション情報を持つコンポーネント
     *
     * 他のSystem（Input/AI/Health等）は m_requested に状態を書くだけでよい。
     * 切り替え・ループ・優先度の処理は AnimationSystem が一元的に行う。
     */
    struct AnimationComponent
    {
        std::unordered_map<constant::AnimationState, AnimationClip> m_clips{};
        constant::AnimationState m_current{ constant::AnimationState::Idle };
        constant::AnimationState m_requested{ constant::AnimationState::Idle };
        int   m_animIndex{ -1 };
        float m_animTime{ 0.0f };
        float m_animTotalTime{ 0.0f };
        bool  m_isCompleted{ false }; // 非ループ再生が終端に達したか
    };
}
