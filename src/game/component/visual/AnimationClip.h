#pragma once
#include "game/constant/AnimationState.h"

namespace game::component::visual
{
	/**
	 * @brief 1アニメーションクリップの再生定義
	 *
	 * AnimationComponent の clips に状態ごとに登録して使う
	 */
	struct AnimationClip
	{
		int m_handle{ -1 }; // mv1アニメーションハンドル（-1は未設定）
		bool m_isLoop{ true }; // 攻撃・被弾・死亡などの単発再生は false
		constant::AnimationState m_onComplete{ constant::AnimationState::Idle }; // 非ループ再生終了後に遷移する状態
		int m_priority{ 0 }; // 割り込み優先度（大きいほど優先。再生中クリップより低い要求は無視）
		float m_speed{ 1.0f }; // 再生速度倍率（1.0が等倍、0.5で半分の速さ）
	};
} // namespace game::component::visual
