#pragma once
#include "IScene.h"
#include <vector>

/* core層のインクルード */
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/SystemManager.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IAnimator.h"
#include "core/interface/IEffectFactory.h"
#include "core/base/EventBus.h"

/* game層のインクルード */
#include "core/interface/IProjectileWindowManager.h"
#include "game/factory/FactoryManager.h"
#include "game/factory/EnemySpawner.h"
#include "game/component/RenderComponent.h"
#include "game/data/PlayerData.h"
#include "game/data/FileEquipmentData.h"
#include "game/event/AudioEventListener.h"
#include "game/factory/ProjectileFactory.h"
#include "game/scene/InGameView.h"
#include <memory>

namespace game::scene
{
	/**
	 * @brief インゲームのシーンクラス
	 */
	class InGame : public IScene
	{
	public:
		/**
		 * @brief InGameのコンストラクタ
		 * @param camera カメラのインターフェース
		 * @param renderer 描画のインターフェース
		 * @param animator アニメーションのインターフェース
		 * @param resourceManager リソース管理のインターフェース
		 * @param inputProvider 入力のインターフェース
		 * @param fileEquipmentData 選択ファイルデータの参照
		 */
		InGame(core::iface::ICamera &camera,
			   core::iface::IRenderer &renderer,
			   core::iface::IAnimator &animator,
			   core::iface::IResourceManager &resourceManager,
			   core::iface::IInputProvider &inputProvider,
			   data::FileEquipmentData &fileEquipmentData);

		/**
		 * @brief シーンの更新処理
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief シーンの描画処理
		 */
		void draw() override;

	private:
		/* コンストラクタで参照する関数 */
		void loadResources();
		void spawnEntities();
		void setupSystems();
		void setupEvents();

		/**
		 * @brief GameManager にリザルトデータを保存する
		 * @param isVictory 勝利かどうか
		 */
		void saveResultData(bool isVictory) noexcept;

		core::ecs::EntityManager 	m_entityManager;
		core::ecs::ComponentManager m_componentManager;
		core::ecs::SystemManager 	m_systemManager;

		core::iface::ICamera          &m_camera;
		core::iface::IRenderer        &m_renderer;
		core::iface::IAnimator        &m_animator;
		core::iface::IResourceManager &m_resourceManager;
		core::iface::IInputProvider   &m_inputProvider;
		data::FileEquipmentData       &m_fileEquipmentData;
		core::iface::IEffectFactory& m_effectFactory;

		game::factory::FactoryManager m_factoryManager;
		game::factory::EnemySpawner m_enemySpawner;
		game::factory::ProjectileFactory m_projectileFactory;
		game::data::PlayerData m_playerData;
		InGameView m_view;

		core::ecs::EntityId m_groundId{core::ecs::INVALID_ENTITY_ID};
		core::ecs::EntityId m_playerId{core::ecs::INVALID_ENTITY_ID};
		std::vector<core::ecs::EntityId> m_enemyIds{};

		core::base::EventBus m_eventBus;

		std::unique_ptr<game::event::AudioEventListener> m_audioEventListener;

		// 弾の見た目として実OSウィンドウを追従表示するマネージャ（Platform層実装）
		std::unique_ptr<core::iface::IProjectileWindowManager> m_projectileWindowManager;

		// 進行トラッキング
		float m_elapsedTime{0.0f};
		int   m_killCount{0};
		float m_totalDamageTaken{0.0f};
	};
} // namespace game::scene