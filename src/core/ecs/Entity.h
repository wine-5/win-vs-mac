#pragma once
#include <cstdint>

namespace core::ecs
{
	/**
	 * @brief EntityのID
	 *
	 * 下位32bitがインデックス、上位32bitが世代（generation）。
	 * 破棄のたびに世代が進むため、インデックスが再利用されても旧IDとは一致しない。
	 */
	using EntityId = std::uint64_t;

	// 0 は無効IDとして扱う（世代は1から始まるので、有効なEntityのidが0になることはない）
	constexpr EntityId INVALID_ENTITY_ID{};

	/// @brief 世代の開始値。0から始めるとインデックス0のidが0（＝無効ID）と衝突するため1にする
	constexpr std::uint32_t INITIAL_GENERATION{ 1 };

	/**
	 * @brief EntityIdからインデックス部を取り出す
	 * @param id EntityID
	 * @return インデックス
	 */
	constexpr std::uint32_t entityIndex(EntityId id) noexcept
	{
		return static_cast<std::uint32_t>(id);
	}

	/**
	 * @brief EntityIdから世代部を取り出す
	 * @param id EntityID
	 * @return 世代
	 */
	constexpr std::uint32_t entityGeneration(EntityId id) noexcept
	{
		return static_cast<std::uint32_t>(id >> 32);
	}

	/**
	 * @brief インデックスと世代からEntityIdを組み立てる
	 * @param index インデックス
	 * @param generation 世代
	 * @return EntityID
	 */
	constexpr EntityId makeEntityId(std::uint32_t index, std::uint32_t generation) noexcept
	{
		return (static_cast<EntityId>(generation) << 32) | index;
	}

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
		EntityId getId() const noexcept { return m_id; }

		/** @brief IDが設定されているか判定（生存確認は EntityManager::isAlive を使う） */
		bool isValid() const noexcept { return m_id != INVALID_ENTITY_ID; }

		/** @brief 等価演算子 */
		bool operator==(const Entity& other) const noexcept { return m_id == other.m_id; }

		/** @brief 不等価演算子 */
		bool operator!=(const Entity& other) const noexcept { return m_id != other.m_id; }

	private:
		EntityId m_id{INVALID_ENTITY_ID};
	};
} // namespace core::ecs