#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"

namespace game::actor
{
	/**
	 * @brief Playerのセットアップを担当するクラス
	 */
	class Player
	{
	public:
		/**
		 * @brief Playerのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager アニメーションハンドル取得用のIResourceManager
		 * @param modelHandle モデルハンドル
		 * @param playerData プレイヤーのデータ
		 */
		Player(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			core::iface::IResourceManager& resourceManager,
			int modelHandle,
			const data::PlayerData& playerData);

		/**
		 * @brief PlayerのEntityIDを取得する
		 * @return EntityID
		 */
		core::ecs::EntityId getId() const noexcept;

	private:
	  /**
	   * @brief 剣を右手のボーンへ装着する（WeaponAttachComponentを付与する）
	   *
	   * 装着先ボーン名はモデルのリグ依存で、見つからない場合は
	   * WeaponAttachSystem が実際のボーン名一覧をログへ出力する
	   * @param componentManager ComponentManagerの参照
	   * @param resourceManager 剣モデル読み込み用のIResourceManager
	   * @param playerScale プレイヤーの拡大率（剣の見た目サイズの逆算に使う）
	   */
	  void attachWeapon(core::ecs::ComponentManager& componentManager,
		  core::iface::IResourceManager& resourceManager,
		  float playerScale);

	  core::ecs::Entity m_entity;
	};
} // namespace game::actor