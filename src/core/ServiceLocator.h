#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "utility/LogUtil.h"

namespace core
{
    /**
     * @brief グローバルなサービスへのアクセスを管理するクラス
     */
    class ServiceLocator
    {
    public:
        template<typename T>
        static void provide(std::unique_ptr<T> service)
        {
            auto key = std::type_index(typeid(T));
            m_services[key] = std::shared_ptr<void>(std::move(service));
        }

        template<typename T>
        static T* get()
        {
            auto key = std::type_index(typeid(T));
            auto it = m_services.find(key);
            if (it == m_services.end())
            {
                LOG_E("ServiceLocator: サービスが登録されていません");
                return nullptr;
            }
            return static_cast<T*>(it->second.get());
        }

        static void clear()
        {
            m_services.clear();
        }

    private:
        static std::unordered_map<std::type_index, std::shared_ptr<void>> m_services;
    };
}