#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "core/data/BlockTable.h"
#include "core/data/PropDefinition.h"

namespace infrastructure::resource::repository
{
	/**
	 * @brief 配置物の種類定義（stageCatalog.jsonのprops[]）の読み込みを担当
	 *
	 * type（id）→ モデルパス・素材実寸・コライダー種別 の解決を提供する。
	 * enemies/boss はエディタ専用の情報のため、ゲーム側が読むpropsのみを保持する。
	 *
	 * ブロックの抽選表だけは stageBalance.json から読む。素材の定義と違って
	 * 繰り返し数値を触る対象なので、モデルパスや大きさと同居させない。
	 * 両者は id で結び付くため、突き合わせはこのクラスが受け持つ。
	 */
	class StageCatalogRepository
	{
	  public:
		/**
		 * @brief StageCatalogRepositoryのコンストラクタ
		 *
		 * stageCatalog.json を読み込み、配置物の種類定義を構築する
		 */
		StageCatalogRepository();

		/**
		 * @brief 配置物の種類IDから定義を取得する
		 * @param type 種類ID（例: "floor_folder"）
		 * @return 配置物の種類定義（存在しない場合はthrow）
		 */
		[[nodiscard]] const core::data::PropDefinition& getProp(std::string_view type) const;

		/**
		 * @brief 破壊可能ブロックの抽選表を取得する
		 * @return 抽選表（blockTableが無い場合は空）
		 */
		[[nodiscard]] const core::data::BlockTable& getBlockTable() const noexcept;

	  private:
		/**
		 * @brief stageBalance.json から抽選表を読み込む
		 *
		 * props を読んだあとに呼ぶこと。表の type が実在する種類かを
		 * その場で突き合わせるため、props が空だと全件エラーになる
		 */
		void loadBlockTable();

		std::unordered_map<std::string, core::data::PropDefinition> m_props{};
		core::data::BlockTable m_blockTable{};
	};
} // namespace infrastructure::resource::repository
