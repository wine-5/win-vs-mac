#pragma once
#include "core/utility/Vector3.h"
#include "core/constant/SeType.h"

namespace game::component::combat
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

		// このEntityが次に与えるダメージの倍率。攻撃力そのものを書き換えると
		// 元の値へ戻せなくなるため、段ごとの強弱はこちらで表す。1.0なら等倍
		float m_damageMultiplier{ 1.0f };

		// クリティカルの発生率（0.0〜1.0）と、発生したときのダメージ倍率。
		// 0なら一度も発生しないので、値を持たない敵は自動的にクリティカルしない
		float m_criticalRate{ 0.0f };
		float m_criticalMultiplier{ 1.0f };

		// 攻撃が届く高さの上限（攻撃者の足元からの相対Y）。地面を叩きつける攻撃のように
		// 「跳んでいれば当たらない」攻撃で使う。0なら高さ無制限（従来動作）
		float m_attackMaxHeight{ 0.0f };

		// このフレームでAttackSystemが実際に攻撃を開始したか。
		// 攻撃間隔の管理はAttackSystem側に一本化しているため、AI Systemが
		// 「攻撃した瞬間」を知りたい場合（攻撃アニメの要求など）はこれを見る
		bool m_justFired{ false };

		// 攻撃開始エフェクトの位置補正（ワールド単位）。手のボーン位置を基準に出すと
		// エフェクトの絵柄によっては高すぎたり低すぎたりするため、その差を吸収する
		core::Vector3 m_effectPositionOffset{};

		// 攻撃開始エフェクトの向き補正（ラジアン）。攻撃を要求した側が「どう振ったか」を
		// ここに入れ、AttackSystem が AttackStartEvent へそのまま載せる。
		// 同じ斬撃エフェクトを縦振りと水平回転で使い分けるために使う
		core::Vector3 m_effectRotationOffset{};

		// 振り始めに鳴らすSE。攻撃を要求した側が「どう振ったか」に応じて入れ、
		// AttackSystem が AttackStartEvent へそのまま載せる。Noneなら無音。
		// 近接コンボの段ごとに振り音を変えるために使う
		core::constant::SeType m_startSeType{ core::constant::SeType::None };
	};
} // namespace game::component::combat