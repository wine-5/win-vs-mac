#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "core/data/PropDefinition.h"

namespace infrastructure::resource::repository
{
	/**
	 * @brief 配置物の種類定義（stageCatalog.jsonのprops[]）の読み込みを担当
	 *
	 * type（id）→ モデルパス・素材実寸・コライダー種別 の解決を提供する。
	 * enemies/boss はエディタ専用の情報のため、ゲーム側が読むpropsのみを保持する。
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

	  private:
		std::unordered_map<std::string, core::data::PropDefinition> m_props{};
	};
} // namespace infrastructure::resource::repository
