#pragma once
#include "FactoryManager.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"
#include <cassert>

namespace game::factory
{
	/**
	 * @brief FactoryManagerを使ったエンティティの初期化処理のみを担当
	 * @note 敵の生成は EnemySpawner に委譲している（初期スポーン・召喚で共有するため）。
	 */
	class FactoryInitializer
	{
	public:
	  /**
	   * @brief コンストラクタ
	   * @param factoryManager FactoryManagerの参照
	   * @param resourceManager IResourceManagerの参照
	   * @param entityManager EntityManagerの参照（Componentのみで完結するEntity生成に使う）
	   * @param componentManager ComponentManagerの参照
	   */
	  FactoryInitializer(
		  FactoryManager& factoryManager,
		  core::iface::IResourceManager& resourceManager,
		  core::ecs::EntityManager& entityManager,
		  core::ecs::ComponentManager& componentManager);

	  /**
	   * @brief Playerを初期化
	   * @param playerData Playerのデータ
	   */
	  void initializePlayer(const data::PlayerData& playerData);

	  /**
	   * @brief stageData.jsonのprops[]から配置物（床・壁など）を一括生成する
	   *
	   * 各propのtypeをstageCatalogで解決し、size÷baseSizeをモデルスケール、
	   * 回転（度）をラジアンへ変換して生成する。
	   */
	  void initializeProps();

	  /**
	   * @brief stageData.jsonのlights[]からステージの点光源を生成する
	   *
	   * 「青い道中 → 白銀のアリーナ」のような明暗の演出をデータ側で組めるようにする。
	   * 生成するのはTransformとLightComponentだけのEntityで、LightSystemが光源を作る。
	   */
	  void initializeLights();

	private:
	  FactoryManager& m_factoryManager;
	  core::iface::IResourceManager& m_resourceManager;
	  core::ecs::EntityManager& m_entityManager;
	  core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::factory