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
	 * 右クリックを押している間は溜め、離した瞬間にカメラ前方へ発射する。
	 * 溜め時間に応じて威力・サイズが上がる（上限は弾定義のchargeMaxTime）。
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
		    core::data::ProjectileMetadata metadata,
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
		/**
		 * @brief 溜め状態に応じた弾を発射する
		 * @param chargeRate 溜め率（0.0＝溜めなし〜1.0＝最大溜め）
		 */
		void fire(float chargeRate);

		core::data::ProjectileMetadata m_metadata{}; // 弾定義（値コピーで保持）
		int m_projectileImageHandle{ -1 };           // 弾のビルボード画像ハンドル
		float m_cooldownTimer{ 0.0f };
		float m_chargeTime{ 0.0f }; // 現在の溜め時間（秒）
		bool m_isCharging{ false }; // 溜め中かどうか
	};
} // namespace game::system
