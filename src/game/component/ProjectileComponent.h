#pragma once
#include "core/utility/Vector3.h"
#include "core/constant/EffectType.h"

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

		// 0より大きければルーレット回転（画面正対のZ軸スピン）で描画する。
		// 値は1ワールド単位進むごとの回転量[rad]（レインボーの演出用）
		float m_spinRollSpeed{ 0.0f };

		// モデルのAABB中心（ローカル・スケール未適用）。原点ズレを打ち消して中心まわりに回すために使う
		core::Vector3 m_spinCenter{ 0.0f, 0.0f, 0.0f };

		// 発射時に再生する演出エフェクト。Noneならエフェクト無し（Safariのタブ弾など）。
		// AttackSystemが初回1回だけこの種別でAttackStartEventを発行する
		core::constant::EffectType m_startEffect{ core::constant::EffectType::None };

		// 発射時の演出（AttackStartEvent）を再生済みか。ProjectileSystemが毎フレーム
		// m_attackRequestedを立て直すため、AttackSystem側でこれを見て初回の1回だけに絞る
		bool m_hasPlayedStartEffect{ false };
	};
} // namespace game::component