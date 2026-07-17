#pragma once

namespace game::component
{
	/**
	 * @brief プレイヤーの溜め攻撃の視覚演出を担当するコンポーネント
	 *
	 * 溜め状態と溜め率を保持し、RangedAttackSystemによって毎フレーム更新される。
	 * PlayerChargeVisualsSystemが読み取り、画面演出（集中線など）を指示する。
	 */
	struct PlayerChargeComponent
	{
		bool m_isCharging{};  // 溜め中かどうか
		float m_chargeRate{}; // 溜め率（0.0 ~ 1.0）
	};
} // namespace game::component
