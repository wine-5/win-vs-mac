#pragma once
#include <cmath>

namespace core
{
	/**
	 * @brief 3次元ベクトル型（DxLibに非依存）
	 */
	struct Vector3
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};

		/** @brief デフォルトコンストラクタ */
		Vector3() = default;
		
		/**
		 * @brief パラメータ付きコンストラクタ
		 * @param x X座標
		 * @param y Y座標
		 * @param z Z座標
		 */
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

		/** @brief ベクトル加算 */
		Vector3 operator+(const Vector3& other) const noexcept { return { x + other.x, y + other.y, z + other.z }; }
		
		/** @brief ベクトル減算 */
		Vector3 operator-(const Vector3& other) const noexcept { return { x - other.x, y - other.y, z - other.z }; }
		
		/** @brief ベクトル加算代入 */
		Vector3& operator+=(const Vector3& other) noexcept { x += other.x; y += other.y; z += other.z; return *this; }

		/** @brief スカラー倍 */
		Vector3 operator*(float scalar) const noexcept
		{
			return { x * scalar, y * scalar, z * scalar };
		}

		/** @brief 符号反転 */
		Vector3 operator-() const noexcept
		{
			return { -x, -y, -z };
		}

		/**
		 * @brief 長さの2乗を返す
		 *
		 * 距離の大小比較だけが必要な場面では平方根を避けられるためこちらを使う。
		 * @return 長さの2乗
		 */
		[[nodiscard]] float lengthSq() const noexcept
		{
			return x * x + y * y + z * z;
		}

		/** @brief 長さを返す */
		[[nodiscard]] float length() const noexcept
		{
			return std::sqrt(lengthSq());
		}

		/**
		 * @brief 正規化したベクトルを返す
		 * @return 長さ1のベクトル（長さ0の場合はゼロベクトル）
		 */
		[[nodiscard]] Vector3 normalized() const noexcept
		{
			const float len{ length() };
			return len > 0.0f ? Vector3{ x / len, y / len, z / len } : Vector3{};
		}
	};
} // namespace core