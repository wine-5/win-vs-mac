#pragma once

namespace game::component::ai
{
	/**
	 * @brief 遠距離維持型AI用のコンポーネント
	 *
	 * プレイヤーから一定距離を保ちながら遠距離攻撃（弾発射等）をする敵用。
	 * 近づきすぎたら後退、遠ければ接近する。必要に応じてホバー（浮遊）する。
	 * RangeKeepAISystemが本コンポーネントの有無で「この敵は距離維持型」と判定する。
	 */
	struct RangeKeepAIComponent
	{
		float m_preferredDistanceMin{ 0.0f }; // この距離より近づかれたら後退する
		float m_preferredDistanceMax{ 0.0f }; // この距離より遠ければ接近する
		float m_hoverHeight{ 0.0f };          // 浮遊高度（0なら地上。Safariなど空を飛ぶ敵用）
	};
} // namespace game::component::ai
