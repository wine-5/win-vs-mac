#pragma once
#include "Entity.h"
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace game::ecs
{
	struct IComponent
	{
		virtual ~IComponent() = default;
	};

	template <typename T>
	class ComponentArray : public IComponent
	{
	public:
		void add(EntityId id, T component)
		{
			m_component[id] = component;
		}

		T& get(EntityId id)
		{
			return m_component[id];
		}

		void remove(EntityId id)
		{
			m_component.erase(id);
		}

		bool has(EntityId id) const
		{
			return m_component.find(id) != m_component.end();
		}

	private:
		std::unordered_map<EntityId, T> m_component;
	};

	class ComponentManager
	{
	public:
		template<typename T>
		void add(EntityId id, T component)
		{
			getComponentArray<T>()->add(id, component);
		}
		template<typename T>
		T& get(EntityId id)
		{
			return getComponentArray<T>()->get(id);
		}
		template<typename T>
		void remove(EntityId id)
		{
			getComponentArray<T>()->remove(id);
		}

		template<typename T>
		bool has(EntityId id)
		{
			return getComponentArray<T>()->has(id);
		}

		void removeAll(EntityId id)
		{
			for (auto& [type, array] : m_componentArrays)
			{
				// 各コンポーネントのremoveを参照する
			}
		}


	private:
		template<typename T>
		ComponentArray<T>* getComponentArray()
		{
			std::type_index type = typeid(T);

			if (m_componentArrays.find(type) == m_componentArrays.end())
				m_componentArrays[type] = std::make_unique<ComponentArray<T>>();

			return static_cast<ComponentArray<T>*>(m_componentArrays[type].get());
		}

		std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_componentArrays;
	};
}