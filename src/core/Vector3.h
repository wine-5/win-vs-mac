#pragma once

namespace core
{
    /**
     * @brief 3次元ベクトル型（DxLibに非依存）
     */
    struct Vector3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vector3() = default;
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vector3 operator+(const Vector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
        Vector3 operator-(const Vector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
        Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    };
}