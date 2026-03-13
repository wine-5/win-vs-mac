#pragma once

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
		Vector3 operator+(const Vector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
		
		/** @brief ベクトル減算 */
		Vector3 operator-(const Vector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
		
		/** @brief ベクトル加算代入 */
		Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
	};
}