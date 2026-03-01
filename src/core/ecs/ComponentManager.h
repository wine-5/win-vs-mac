#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
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
                array->remove(id);
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