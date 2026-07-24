#pragma once
#include "core/utility/Vector3.h"

namespace game::component::movement
{
	/**
	 * @brief 奈落へ落ちたときに戻る場所を覚えておくコンポーネント
	 *
	 * 落下を検知して直前に立っていた場所へ戻す方式で救済する。
	 *
	 * 深く潜っていくステージなので、落下判定は「絶対的な高さ」ではなく
	 * 「最後に立っていた場所からどれだけ下がったか」で見る。
	 */
	struct FallRecoveryComponent
	{
		core::Vector3 m_lastSafePosition{}; // 最後に接地していた位置
		bool m_hasSafePosition{ false };    // 一度でも接地したか
	};
} // namespace game::component::movement
