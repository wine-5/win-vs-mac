#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

namespace game::utility
{
	/**
	 * @brief その向きへ踏み出しても落ちないかを判定する
	 *
	 * CliffAvoidanceComponent を持つエンティティだけを見張る。持っていなければ常に許可するので、
	 * 「崖で止まる敵か」はコンポーネントの有無だけで決まり、AI側に敵ごとの分岐が要らない。
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 対象のEntityId
	 * @param direction 進もうとしている正規化済み水平方向ベクトル
	 * @return 踏み出してよい場合true（見張り対象外・空中にいる場合もtrue）
	 */
	[[nodiscard]] bool canStepToward(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, const core::Vector3& direction);

	/**
	 * @brief そのXZに立てる床があるかを判定する（移動先の候補を選ぶときに使う）
	 *
	 * 崖チェックと同じ基準で「床の上か」を見る。CliffAvoidanceComponent を持たない
	 * エンティティは見張り対象外なので常にtrueを返す。
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 対象のEntityId
	 * @param x ワールドX座標
	 * @param z ワールドZ座標
	 * @param referenceHeight 基準の高さ（これより上の面は別階層とみなして無視する）
	 * @return 床がある場合true
	 */
	[[nodiscard]] bool hasFootingAt(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, float x, float z, float referenceHeight);
} // namespace game::utility
