#pragma once

namespace game::component
{
	/**
	 * @brief 攻撃を持つコンポーネント
	 */
	struct AttackComponent
	{
		// 0はJSON未設定時に即座に異常検知できるようにするための意図的な初期値
		float m_attackPower{ 0.0f };
		float m_attackRange{ 0.0f };
		float m_attackCooldown{ 0.0f };
		float m_currentCooldown{ 0.0f };
		bool  m_attackRequested{ false };

		// ワインドアップ（溜め）：攻撃要求からダメージ判定までの遅延（秒）。
		// アニメーションの振りが終わってからダメージを与えたい場合に使う。0なら即時判定（従来動作）。
		float m_windupDelay{ 0.0f };
		float m_windupTimer{ 0.0f };
		bool m_windupPending{ false };
	};
} // namespace game::component