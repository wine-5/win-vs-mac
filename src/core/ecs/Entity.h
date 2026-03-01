#pragma once
#include <cstdint>

namespace core::ecs
{
	using EntityId = uint32_t;

	// 0 は無効IDとして扱う
	constexpr EntityId INVALID_ENTITY_ID = 0;

	/** @brief エンティティIDのラッパークラス */
	class Entity
	{
	public:
		explicit Entity(EntityId id) : m_id(id) {}

		EntityId getId() const { return m_id; }
		bool isValid() const { return m_id != INVALID_ENTITY_ID; }

		bool operator==(const Entity& other) const { return m_id == other.m_id; }
		bool operator!=(const Entity& other) const { return m_id != other.m_id; }

	private:
		EntityId m_id;
	};
}