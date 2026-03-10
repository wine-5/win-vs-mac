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
		/**
		 * @brief Entityのコンストラクタ
		 * @param id EntityID
		 */
		explicit Entity(EntityId id) : m_id(id) {}

		/** @brief EntityIDを取得 */
		EntityId getId() const { return m_id; }
		
		/** @brief Entityが有効か判定 */
		bool isValid() const { return m_id != INVALID_ENTITY_ID; }

		/** @brief 等価演算子 */
		bool operator==(const Entity& other) const { return m_id == other.m_id; }
		
		/** @brief 不等価演算子 */
		bool operator!=(const Entity& other) const { return m_id != other.m_id; }

	private:
		EntityId m_id;
	};
}