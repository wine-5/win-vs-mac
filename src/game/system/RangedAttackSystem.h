#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/data/ProjectileMetadata.h"
#include "game/factory/ProjectileFactory.h"

namespace game::system
{
	/**
	 * @brief プレイヤーの遠距離攻撃（Window投擲）の発射を担うSystem
	 *
	 * 発射入力とクールダウンを見て、カメラ前方へ弾を発射する。
	 * 弾のパラメータは projectileData.json（ProjectileMetadata）から与えられ、
	 * 弾の生成自体は ProjectileFactory に委譲する（敵の遠距離でも同じFactoryを使う）。
	 */
	class RangedAttackSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief RangedAttackSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param playerId プレイヤーのEntityID
		 * @param projectileFactory 弾生成ファクトリの参照
		 * @param metadata 弾定義（projectileData.jsonから取得したもの）
		 * @param projectileImageHandle 弾のビルボード画像ハンドル（-1なら仮スフィア描画）
		 */
		RangedAttackSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId,
		    factory::ProjectileFactory& projectileFactory,
		    const core::data::ProjectileMetadata& metadata,
		    int projectileImageHandle);

		/**
		 * @brief 発射入力に応じて弾を発射する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId{};
		factory::ProjectileFactory& m_projectileFactory;
		core::data::ProjectileMetadata m_metadata{}; // 弾定義（値コピーで保持）
		int m_projectileImageHandle{ -1 };           // 弾のビルボード画像ハンドル
		float m_cooldownTimer{ 0.0f };
	};
} // namespace game::system
