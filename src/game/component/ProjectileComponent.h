#pragma once

namespace game::component
{
	/**
	 * @brief 投射物（弾）に固有のデータを持つコンポーネント
	 *
	 * 当たり判定・ダメージ・移動・陣営は既存Component（Attack/Velocity/Tag）を流用するため、
	 * ここには弾に固有の「寿命」だけを持たせる。
	 */
	struct ProjectileComponent
	{
		float m_lifetime{}; // 投擲物の寿命（0以下で自動消滅する）
	};
} // namespace game::component