#pragma once
#include <vector>
#include <random>
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/data/ProjectileMetadata.h"
#include "game/factory/ProjectileFactory.h"

namespace game::system::ai
{
	/**
	 * @brief 距離維持型敵（Safari等）の遠距離攻撃（弾発射）を担うSystem
	 *
	 * RangeKeepAIComponentを持つ敵ごとに発射クールダウンを管理し、
	 * プレイヤーが索敵範囲内かつクールダウン完了ならプレイヤーへ向けて弾を発射する。
	 * 弾のパラメータは projectileData.json（ProjectileMetadata）から与えられ、
	 * 見た目は複数のモデルからランダムに選ぶ（タブ弾の3種など）。
	 *
	 * 敵の接触ダメージ（AttackComponent/attackPower）とは独立した系統であり、
	 * AttackSystemの近接判定には一切干渉しない。
	 */
	class EnemyRangedAttackSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief EnemyRangedAttackSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param projectileFactory 弾生成ファクトリの参照
		 * @param metadata 弾定義（projectileData.jsonから取得したもの）
		 * @param modelHandles 弾の見た目に使うモデルハンドル群（この中からランダムに選ぶ）
		 */
		EnemyRangedAttackSystem(core::ecs::ComponentManager& componentManager,
		    factory::ProjectileFactory& projectileFactory,
		    core::data::ProjectileMetadata metadata,
		    std::vector<int> modelHandles);

		/**
		 * @brief 距離維持型敵の弾発射処理を実行する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief モデルハンドル群からランダムに1つ選ぶ（空なら-1）
		 * @return モデルハンドル
		 */
		int pickRandomModel();

		core::ecs::ComponentManager& m_componentManager;
		factory::ProjectileFactory& m_projectileFactory;
		core::data::ProjectileMetadata m_metadata{}; // 弾定義（値コピーで保持）
		std::vector<int> m_modelHandles{};           // 弾の見た目候補
		std::mt19937 m_rng{ std::random_device{}() };
	};
} // namespace game::system::ai
