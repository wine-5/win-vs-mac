#pragma once

namespace game::constant
{
	/**
	 * @brief ゲーム内に存在しうるアニメーション状態の全集合
	 *
	 * 全キャラがこの全状態を持つわけではない。各キャラは自分が使う状態だけを
	 * AnimationComponent の clips に登録する（未登録状態への要求は無視される）。
	 * Attack1 等の抽象名の意味付けは登録側が決める（例: PlayerではSwordSlash、
	 * XcodeではGroundSlam）
	 */
	enum class AnimationState
	{
		Idle,
		Walk,
		Run,
		Attack1,
		Attack2,
		Hit,
		Dying,
		Jump,
	};

	/**
	 * @brief アニメーションの割り込み優先度の定数
	 *
	 * 再生中クリップより低い優先度の要求は再生完了まで無視される。
	 * この大小関係はゲーム全体のルールなので、クリップ登録時は必ずここを使う
	 */
	namespace animation_priority
	{
		constexpr int DYING      = 100; // 死亡（何にも割り込まれない）
		constexpr int HIT        = 50;  // 被弾（攻撃をキャンセルする）
		constexpr int ATTACK     = 30;  // 攻撃
		constexpr int JUMP       = 20;  // ジャンプ
		constexpr int LOCOMOTION = 0;   // 移動系（Idle/Walk/Run、互いに自由に遷移）
	}

	/**
	 * @brief アニメーション状態をログ出力用の文字列に変換
	 * @param state 変換する状態
	 * @return 状態名の文字列
	 */
	inline constexpr const char* toString(AnimationState state)
	{
		switch (state)
		{
		case AnimationState::Idle:    return "Idle";
		case AnimationState::Walk:    return "Walk";
		case AnimationState::Run:     return "Run";
		case AnimationState::Attack1: return "Attack1";
		case AnimationState::Attack2: return "Attack2";
		case AnimationState::Hit:     return "Hit";
		case AnimationState::Dying:   return "Dying";
		case AnimationState::Jump:    return "Jump";
		default:                      return "Unknown";
		}
	}
}
