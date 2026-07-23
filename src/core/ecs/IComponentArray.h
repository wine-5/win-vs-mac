#pragma once
#include "Entity.h"

namespace core::ecs
{
	/**
	 * @brief ComponentArray の型消去用インターフェース
	 *
	 * ComponentManager が「型の違う ComponentArray」を1つのコンテナへまとめて
	 * 保持するための基底。ゲーム側のコンポーネント（AIComponent 等）とは無関係。
	 */
	class IComponentArray
	{
	  public:
		virtual ~IComponentArray() = default;

		/**
		 * @brief EntityのComponentを削除する
		 * @param id EntityID
		 */
		virtual void remove(EntityId id) = 0;
	};
} // namespace core::ecs