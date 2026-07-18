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
	 * @brief 弾の見た目1種類分（モデルハンドルと当たり判定半径のペア）
	 *
	 * 半径はモデルの実寸から自動計算した値、または手動指定値が入る。
	 */
	struct RangedProjectileVisual
	{
		int m_modelHandle{ -1 }; // 3Dモデルハンドル
		float m_radius{ 0.0f };  // 当たり判定半径
	};

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
		 * @param visuals 弾の見た目候補（モデルと半径のペア群。この中からランダムに選ぶ）
		 */
		EnemyRangedAttackSystem(core::ecs::ComponentManager& componentManager,
		    factory::ProjectileFactory& projectileFactory,
		    core::data::ProjectileMetadata metadata,
		    std::vector<RangedProjectileVisual> visuals);

		/**
		 * @brief 距離維持型敵の弾発射処理を実行する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief 見た目候補からランダムに1つ選ぶ（空ならデフォルト値）
		 * @return 選ばれた見た目（モデルハンドルと半径）
		 */
		RangedProjectileVisual pickRandomVisual();

		core::ecs::ComponentManager& m_componentManager;
		factory::ProjectileFactory& m_projectileFactory;
		core::data::ProjectileMetadata m_metadata{};     // 弾定義（値コピーで保持）
		std::vector<RangedProjectileVisual> m_visuals{}; // 弾の見た目候補
		std::mt19937 m_rng{ std::random_device{}() };
	};
} // namespace game::system::ai
