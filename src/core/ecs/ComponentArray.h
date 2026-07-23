#pragma once
#include <cassert>
#include <unordered_map>
#include <vector>
#include "Entity.h"
#include "IComponent.h"

namespace core::ecs
{
	/**
	 * @brief EntityIdとComponentを紐付けて管理するコンテナクラス
	 */
	template <typename T>
	class ComponentArray : public IComponent
	{
	public:
		/**
		 * @brief EntityにComponentを追加する
		 * @param id EntityID
		 * @param component 追加するComponent
		 */
		void add(EntityId id, T component)
		{
			m_component[id] = component;
		}

		/**
		 * @brief EntityのComponentを取得する
		 *
		 * 持っていないEntityを指定した場合はプログラムの誤りなのでassertで検出する。
		 * 有無が不定な場面では tryGet() を使うこと。
		 * @param id EntityID
		 * @return Componentの参照
		 */
		T& get(EntityId id)
		{
			auto it{ m_component.find(id) };
			assert(it != m_component.end() && "ComponentArray::get(): Entityが該当Componentを持っていません");
			return it->second;
		}

		/**
		 * @brief EntityのComponentを取得する（持っていなければnullptr）
		 *
		 * has()→get() の二連ハッシュ検索を1回にまとめる用途にも使う。
		 * @param id EntityID
		 * @return Componentのポインタ（持っていない場合nullptr）
		 */
		[[nodiscard]] T* tryGet(EntityId id)
		{
			auto it{ m_component.find(id) };
			return it != m_component.end() ? &it->second : nullptr;
		}

		/**
		 * @brief EntityのComponentを削除する
		 * @param id EntityID
		 */
		void remove(EntityId id) override
		{
			m_component.erase(id);
		}

		/**
		 * @brief Entityが指定Componentを持っているか判定する
		 * @param id EntityID
		 * @return Componentを持っている場合true
		 */
		bool has(EntityId id) const
		{
			return m_component.find(id) != m_component.end();
		}

		/**
		 * @brief このComponentを持つ全EntityのIDを取得する
		 * @return EntityIDのベクター
		 */
		std::vector<EntityId> getAllEntities() const
		{
			std::vector<EntityId> entities;
			entities.reserve(m_component.size());
			for (const auto& [id, _] : m_component)
			{
				entities.push_back(id);
			}
			return entities;
		}

	private:
		std::unordered_map<EntityId, T> m_component;
	};
} // namespace core::ecs