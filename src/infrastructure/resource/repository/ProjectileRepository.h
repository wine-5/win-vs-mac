#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "core/data/ProjectileMetadata.h"

namespace infrastructure::resource::repository
{
	/**
	 * @brief 弾定義（projectileData.json）の読み込みを担当
	 */
	class ProjectileRepository
	{
	  public:
		/**
		 * @brief ProjectileRepositoryのコンストラクタ
		 *
		 * projectileData.json を読み込み、弾定義を構築する
		 */
		ProjectileRepository();

		/**
		 * @brief 弾IDから弾定義を取得する
		 * @param projectileId 弾ID（例: "player_window"）
		 * @return 弾定義（存在しない場合はthrow）
		 */
		[[nodiscard]] const core::data::ProjectileMetadata& getMetadata(std::string_view projectileId) const;

	  private:
		std::unordered_map<std::string, core::data::ProjectileMetadata> m_metadata{};
	};
} // namespace infrastructure::resource::repository
