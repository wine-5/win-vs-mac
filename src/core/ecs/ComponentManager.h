#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include "Entity.h"
#include "IComponentArray.h"
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
		 * @brief EntityのComponentを取得する（持っていなければnullptr）
		 *
		 * has()→get() と2回ハッシュ検索する代わりに1回で済ませたい場面で使う。
		 * @tparam T Componentの型
		 * @param id EntityID
		 * @return Componentのポインタ（持っていない場合nullptr）
		 */
		template <typename T>
		[[nodiscard]] T* tryGet(EntityId id)
		{
			return getComponentArray<T>()->tryGet(id);
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
	  // find→operator[] と2回ハッシュ検索していたのを1回にまとめる。
	  // get/has/tryGet は毎フレーム大量に呼ばれるため、ここの1回分がそのまま効いてくる
	  template <typename T>
	  ComponentArray<T>* getComponentArray()
	  {
		  std::type_index type{ typeid(T) };
		  auto it{ m_componentArrays.find(type) };
		  if (it == m_componentArrays.end())
			  it = m_componentArrays.emplace(type, std::make_unique<ComponentArray<T>>()).first;
		  return static_cast<ComponentArray<T>*>(it->second.get());
		}

		std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> m_componentArrays;
	};
} // namespace core::ecs