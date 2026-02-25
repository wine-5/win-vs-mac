#pragma once
#include <unordered_map>
#include "Entity.h"
#include "IComponent.h"

namespace game::ecs
{
    // EntityIdとComponentを紐付けて管理する
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
}