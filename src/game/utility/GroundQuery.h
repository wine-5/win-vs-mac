#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include <optional>

namespace game::utility
{
	/**
	 * @brief 足場の問い合わせ結果
	 */
	struct GroundHit
	{
		float m_height{ 0.0f };                     // その位置での天面の高さ
		core::Vector3 m_normal{ 0.0f, 1.0f, 0.0f }; // 天面の法線（滑り方向の算出に使う）
		core::ecs::EntityId m_surfaceId{};          // 見つかった面のEntityID
	};

	/**
	 * @brief 傾いた天面の、指定XZ位置での高さを求める
	 * @param componentManager ComponentManagerの参照
	 * @param surfaceId 面のEntityID（GroundSurfaceComponentを持つもの）
	 * @param x ワールドX座標
	 * @param z ワールドZ座標
	 * @param outHeight 求まった高さの格納先
	 * @param outNormal 天面の法線の格納先
	 * @return XZが面の範囲内で高さが求まった場合true
	 */
	[[nodiscard]] bool surfaceTopAt(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId surfaceId, float x, float z, float& outHeight, core::Vector3& outNormal);

	/**
	 * @brief 指定XZの足元にある面のうち、最も高いものを探す
	 *
	 * 接地にも「その先に床があるか」の先読みにも使えるよう、天井（これより上の面は無視する高さ）を
	 * 呼び出し側から与える形にしている。別階層の床を拾わないための線引きがここで決まる。
	 * @param componentManager ComponentManagerの参照
	 * @param x ワールドX座標
	 * @param z ワールドZ座標
	 * @param ceiling この高さより上にある面は無視する
	 * @return 見つかった面の情報。無ければnullopt（＝そこは奈落）
	 */
	[[nodiscard]] std::optional<GroundHit> findGround(core::ecs::ComponentManager& componentManager,
	    float x, float z, float ceiling);
} // namespace game::utility
