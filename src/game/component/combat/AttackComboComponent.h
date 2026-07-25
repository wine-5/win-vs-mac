#pragma once

namespace game::component::combat
{
	/**
	 * @brief 近接攻撃の連携（コンボ）の進行状態を持つコンポーネント
	 *
	 * 1段目を出してから m_inputWindow 秒以内に攻撃を押し直すと次の段へ進み、
	 * 過ぎていれば1段目へ戻る。段ごとにどのアニメーションを出すかは
	 * PlayerAttackComboSystem が決める。
	 */
	struct AttackComboComponent
	{
		// 次の段へ進める入力受付時間（秒）。攻撃を出した瞬間から数え始める。
		// 攻撃のクールダウンより短いと次段を押せる猶予が無くなるため、
		// クールダウンより長い値にすること
		float m_inputWindow{ 1.0f };

		// 現在の段数（0=攻撃していない、1=1段目、2=2段目）
		int m_stage{ 0 };

		// 受付時間の残り。0以下になった時点で段数を0へ戻す
		float m_windowTimer{ 0.0f };
	};
} // namespace game::component::combat
