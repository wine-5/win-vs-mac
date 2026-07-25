#pragma once
#include "core/utility/Vector3.h"
#include <cmath>

namespace core::utility
{
	/**
	 * @brief オイラー角でベクトルを回転する（X→Y→Zの順に適用）
	 *
	 * 適用順はDxLibの MV1SetRotationXYZ と揃えること。見た目とロジック（接地計算など）で
	 * 回転がズレると、坂の上で足元が合わなくなる。
	 * @param v 回転させるベクトル
	 * @param rot オイラー角（ラジアン）
	 * @return 回転後のベクトル
	 */
	inline Vector3 rotateEulerXYZ(const Vector3& v, const Vector3& rot) noexcept
	{
		const float cx{ std::cos(rot.x) }, sx{ std::sin(rot.x) };
		const float cy{ std::cos(rot.y) }, sy{ std::sin(rot.y) };
		const float cz{ std::cos(rot.z) }, sz{ std::sin(rot.z) };

		// X軸まわり
		const float x1{ v.x };
		const float y1{ v.y * cx - v.z * sx };
		const float z1{ v.y * sx + v.z * cx };
		// Y軸まわり
		const float x2{ x1 * cy + z1 * sy };
		const float y2{ y1 };
		const float z2{ -x1 * sy + z1 * cy };
		// Z軸まわり
		return Vector3{ x2 * cz - y2 * sz, x2 * sz + y2 * cz, z2 };
	}

	/**
	 * @brief rotateEulerXYZ の逆回転（Z→Y→Xの順に逆回転を適用）
	 *
	 * ワールド座標を配置物のローカル座標へ戻し、箱の範囲内かを判定するのに使う。
	 * @param v 回転を戻したいベクトル
	 * @param rot 元の回転に使ったオイラー角（ラジアン）
	 * @return 逆回転後のベクトル
	 */
	inline Vector3 inverseRotateEulerXYZ(const Vector3& v, const Vector3& rot) noexcept
	{
		const float cx{ std::cos(rot.x) }, sx{ std::sin(rot.x) };
		const float cy{ std::cos(rot.y) }, sy{ std::sin(rot.y) };
		const float cz{ std::cos(rot.z) }, sz{ std::sin(rot.z) };

		// Z軸まわり（逆）
		const float x1{ v.x * cz + v.y * sz };
		const float y1{ -v.x * sz + v.y * cz };
		const float z1{ v.z };
		// Y軸まわり（逆）
		const float x2{ x1 * cy - z1 * sy };
		const float y2{ y1 };
		const float z2{ x1 * sy + z1 * cy };
		// X軸まわり（逆）
		return Vector3{ x2, y2 * cx + z2 * sx, -y2 * sx + z2 * cx };
	}
} // namespace core::utility
