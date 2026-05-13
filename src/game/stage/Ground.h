#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

// 前方宣言
namespace game::data { class GroundData; }

namespace game::stage
{
	/**
	 * @brief 地面のオブジェクト
	 */
	class Ground
	{
	public:
		/**
		 * @brief Groundのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param modelHandle モデルハンドル
		 * @param groundData Groundのデータ
		 */
		Ground(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			int modelHandle,
			const game::data::GroundData& groundData);

		/**
		 * @brief GroundのEntityIDを取得する
		 * @return EntityID
		 */
		core::ecs::EntityId getId() const noexcept;

	private:
		core::ecs::Entity m_entity;
	};
}