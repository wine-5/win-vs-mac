#pragma once
#include <unordered_map>
#include "game/component/visual/AnimationClip.h"
#include "game/constant/AnimationState.h"

namespace game::component::visual
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

		/**
		 * @brief アニメーション状態を要求する（未処理の要求より優先度が高い場合のみ通す）
		 *
		 * m_requested を直接書くと、AnimationSystem が処理する前に別のSystemが
		 * 上書きしてしまい、要求がSystemの実行順に左右される。
		 * （例: MoveSystemが毎フレーム出すIdleが、AttackSystemが出したAttack1を消す）
		 * 本メソッドを通せば、同一フレーム内では優先度の高い要求が残る。
		 * @param state 要求する状態（未登録の状態は無視される）
		 */
		void request(constant::AnimationState state)
		{
			auto requestedIt{ m_clips.find(state) };
			if (requestedIt == m_clips.end())
				return;

			// 未処理の要求（m_requested != m_current）があり、それより優先度が
			// 低いなら通さない
			if (m_requested != m_current)
			{
				auto pendingIt{ m_clips.find(m_requested) };
				if (pendingIt != m_clips.end() &&
				    pendingIt->second.m_priority > requestedIt->second.m_priority)
					return;
			}

			m_requested = state;
		}
	};
} // namespace game::component::visual
