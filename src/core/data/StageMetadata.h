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
		float m_rotationY{}; // 初期の向き（度数法）
	};

	/**
	 * @brief 配置物1つ分の定義（床・壁・柱などのステージ構造物）
	 *
	 * typeはstageCatalog.jsonのprops[].idを参照する。core層はカタログの内容を
	 * 知らないため、モデルパスやbaseSizeへの解決はinfrastructure層の責務とする。
	 * rotationは度数法・sizeは実寸（ユニット）で保持し、モデルスケールへの変換は
	 * 生成側（game層）が行う。
	 */
	struct PropMetadata
	{
		std::string m_type{};
		core::Vector3 m_position{};
		core::Vector3 m_rotation{}; // 度数法。坂はX/Zを使う
		core::Vector3 m_size{};     // 実寸（ユニット）。モデルスケールではない
	};

	/**
	 * @brief プレイヤーの開始位置
	 */
	struct PlayerStartMetadata
	{
		core::Vector3 m_position{};
		float m_rotationY{}; // 度数法
	};

	/**
	 * @brief ステージに置く点光源1つ分の定義
	 *
	 * 「青い道中 → 白銀のアリーナ」のような明暗・色の演出を、
	 * コードではなくステージデータ側で組み立てるために持つ。
	 */
	struct LightMetadata
	{
		core::Vector3 m_position{};
		float m_range{ 1000.0f };
		int m_r{ 255 };
		int m_g{ 255 };
		int m_b{ 255 };
	};

	/**
	 * @brief ステージの配置定義
	 */
	struct StageMetadata
	{
		PlayerStartMetadata m_playerStart{};
		std::vector<PropMetadata> m_props{};
		std::vector<LightMetadata> m_lights{};
		std::vector<SpawnMetadata> m_spawns{};
		SpawnMetadata m_mac{};
	};
} // namespace core::data
