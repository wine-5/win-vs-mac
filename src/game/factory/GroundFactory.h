#pragma once
#include "IFactory.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IResourceManager.h"
#include "game/stage/Ground.h"
#include "game/data/GroundData.h"
#include <memory>
#include <vector>

namespace game::factory
{
	/**
	 * @brief Groundオブジェクトの生成と寿命管理を担当
	 */
	class GroundFactory : public IFactory
	{
	public:
		/**
		 * @brief GroundFactoryのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager IResourceManagerの参照
		 */
		GroundFactory(
			core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			core::iface::IResourceManager& resourceManager);

		/**
		 * @brief Groundオブジェクトを生成する
		 * @param modelHandle モデルハンドル
		 * @param groundData Groundのデータ
		 * @return 生成したGroundのEntityId
		 */
		core::ecs::EntityId create(int modelHandle, const data::GroundData& groundData);

		/**
		 * @brief 指定EntityIdのGroundを取得する
		 * @param id EntityId
		 * @return Groundオブジェクトのポインタ（見つからない場合nullptr）
		 */
		stage::Ground* getGroundById(core::ecs::EntityId id) const;

		/**
		 * @brief 全てのGroundを取得する
		 * @return 全Groundのポインタ配列
		 */
		std::vector<stage::Ground*> getAllGrounds() const;

		/**
		 * @brief 生成したGroundの数を取得する
		 * @return Groundの総数
		 */
		size_t getCount() const;

	private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;

		std::vector<std::unique_ptr<stage::Ground>> m_grounds;
	};
} // namespace game::factory