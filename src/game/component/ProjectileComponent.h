#pragma once
#include "core/utility/Vector3.h"

namespace game::component
{
	/**
	 * @brief 投射物（弾）に固有の実行時状態を持つコンポーネント
	 *
	 * 当たり判定・ダメージ・移動・陣営は既存Component（Attack/Velocity/Tag）を流用する。
	 * 弾種ごとの定義値はProjectileMetadata（JSON）が持ち、
	 * ここには1発ごとに変化する「残り寿命」だけを持たせる。
	 */
	struct ProjectileComponent
	{
		float m_remainingLifetime{};     // 残り寿命（毎フレーム減算し、0以下で自動消滅する）
		core::Vector3 m_spawnPosition{}; // 発射位置（実ウィンドウの出現ディレイ＝発射者から離れたか判定に使う）
	};
} // namespace game::component