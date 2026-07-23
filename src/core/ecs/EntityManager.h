#pragma once
#include <cstdint>
#include <vector>
#include "Entity.h"

namespace core::ecs
{
	/**
	 * @brief EntityのIdの発行、回収を担当
	 *
	 * インデックスは再利用するが、破棄のたびに世代を進めるため、
	 * 再利用されたインデックスのEntityが破棄済みの旧IDと一致することはない。
	 * これにより「破棄済みEntityのIDを保持したまま参照する」事故を isAlive() で検出できる。
	 */
	class EntityManager
	{
	public:
	  /**
	   * @brief Entityを生成する
	   * @return 生成したEntity
	   */
	  Entity create()
	  {
		  std::uint32_t index{};
		  if (!m_freeIndices.empty())
		  {
			  index = m_freeIndices.back();
			  m_freeIndices.pop_back();
		  }
		  else
		  {
			  index = static_cast<std::uint32_t>(m_generations.size());
			  m_generations.push_back(INITIAL_GENERATION);
		  }
		  return Entity{ makeEntityId(index, m_generations[index]) };
		}

		/**
		 * @brief Entityを破棄する
		 *
		 * 世代を進めることで、同じインデックスを指す旧IDを恒久的に無効化する。
		 * 既に破棄済み（世代が一致しない）の場合は何もしない。
		 * @param entity 破棄するEntity
		 */
		void destroy(Entity entity)
		{
			const std::uint32_t index{ entityIndex(entity.getId()) };
			if (index >= m_generations.size())
				return;
			if (m_generations[index] != entityGeneration(entity.getId()))
				return;

			++m_generations[index];
			// 世代が一周した場合、インデックス0のidが0（無効ID）になるのを避ける
			if (m_generations[index] == 0)
				m_generations[index] = INITIAL_GENERATION;

			m_freeIndices.push_back(index);
		}

		/**
		 * @brief Entityが生存しているか判定する
		 * @param entity 判定するEntity
		 * @return 生存している場合true
		 */
		[[nodiscard]] bool isAlive(Entity entity) const noexcept
		{
			const std::uint32_t index{ entityIndex(entity.getId()) };
			return index < m_generations.size() && m_generations[index] == entityGeneration(entity.getId());
		}

		/**
		 * @brief EntityIdが生存しているか判定する
		 * @param id 判定するEntityID
		 * @return 生存している場合true
		 */
		[[nodiscard]] bool isAlive(EntityId id) const noexcept
		{
			return isAlive(Entity{ id });
		}

	private:
	  // インデックスごとの現在の世代
	  std::vector<std::uint32_t> m_generations{};
	  // 再利用可能なインデックス
	  std::vector<std::uint32_t> m_freeIndices{};
	};
} // namespace core::ecs
