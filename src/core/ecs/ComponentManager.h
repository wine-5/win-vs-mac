#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include "Entity.h"
#include "IComponent.h"
#include "ComponentArray.h"

namespace core::ecs
{
	/**
	 * @brief 全ComponentArrayを型ごとに管理するECSマネージャ
	 */
	class ComponentManager
	{
	public:
		/**
		 * @brief EntityにComponentを追加する
		 * @tparam T Componentの型
		 * @param id EntityID
		 * @param component 追加するComponent
		 */
		template<typename T>
		void add(EntityId id, T component)
		{
			getComponentArray<T>()->add(id, component);
		}

		/**
		 * @brief EntityのComponentを取得する
		 * @tparam T Componentの型
		 * @param id EntityID
		 * @return Componentの参照
		 */
		template<typename T>
		T& get(EntityId id)
		{
			return getComponentArray<T>()->get(id);
		}

		/**
		 * @brief EntityのComponentを削除する
		 * @tparam T Componentの型
		 * @param id EntityID
		 */
		template<typename T>
		void remove(EntityId id)
		{
			getComponentArray<T>()->remove(id);
		}

		/**
		 * @brief Entityが指定Componentを持っているか判定する
		 * @tparam T Componentの型
		 * @param id EntityID
		 * @return Componentを持っている場合true
		 */
		template<typename T>
		bool has(EntityId id)
		{
			return getComponentArray<T>()->has(id);
		}

		/**
		 * @brief 指定したComponentを持つ全EntityのIDを取得する
		 * @tparam T Componentの型
		 * @return EntityIDのベクター
		 */
		template<typename T>
		std::vector<EntityId> getAllEntities()
		{
			return getComponentArray<T>()->getAllEntities();
		}

		/**
		 * @brief Entityの全Componentを削除する
		 * @param id EntityID
		 */
		void removeAll(EntityId id)
		{
			for (auto& [type, array] : m_componentArrays)
			{
				array->remove(id);
			}
		}

	private:
		template<typename T>
		ComponentArray<T>* getComponentArray()
		{
			std::type_index type{typeid(T)};
			if (m_componentArrays.find(type) == m_componentArrays.end())
				m_componentArrays[type] = std::make_unique<ComponentArray<T>>();
			return static_cast<ComponentArray<T>*>(m_componentArrays[type].get());
		}

		std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_componentArrays;
	};
} // namespace core::ecs