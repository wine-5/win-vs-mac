#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::data
{
	class EnemyData;
}

namespace game::actor
{
	/**
	 * @brief EnemyDataのbehaviorsレシピに従い、対応するAIコンポーネントを付与する
	 *
	 * 敵種ごとの派生クラスに代わり、JSONの "behaviors" 配列で「積む振る舞い」を組み合わせる。
	 * 各振る舞い名に対応するインストーラが、必要なパラメータをEnemyDataやMac定義から読んで
	 * コンポーネントを追加する。未知の名前はログを出して無視する。
	 *
	 * 対応する振る舞い名：
	 *   "meleeChase" … 近接追跡（MeleeChaseAIComponent）
	 *   "rangeKeep"  … 距離維持＋周回＋遠距離発射（RangeKeepAIComponent）
	 *   "patrol"     … 索敵範囲外の徘徊（PatrolComponent）
	 *   "boss"       … FSM駆動ボス（MacAIComponent）。攻撃クールダウンはFSMに委ねる
	 *
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 対象の敵EntityId
	 * @param enemyData 敵データ（パラメータ・レシピの供給元）
	 */
	void installEnemyBehaviors(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, const data::EnemyData& enemyData);
} // namespace game::actor
