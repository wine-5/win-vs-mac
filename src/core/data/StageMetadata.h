#pragma once
#include <string>
#include <vector>
#include "core/utility/Vector3.h"

namespace core::data
{
	/**
	 * @brief 敵1体分のスポーン定義
	 *
	 * core層はゲーム固有の敵種を知らないため、typeは文字列のまま保持する。
	 * 列挙型への変換はgame層の責務
	 */
	struct SpawnMetadata
	{
		std::string m_type{};
		core::Vector3 m_position{};
	};

	/**
	 * @brief ステージの配置定義
	 */
	struct StageMetadata
	{
		std::vector<SpawnMetadata> m_spawns{};
		SpawnMetadata m_boss{};
	};
}
