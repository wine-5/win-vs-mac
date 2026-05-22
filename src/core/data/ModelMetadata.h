#pragma once
#include "core/utility/Vector3.h"
#include <string>
#include <unordered_map>

namespace core::data
{
	/**
	 * @brief モデルのメタデータ（JSONに依存しない純粋な構造体）
	 */
	struct ModelMetadata
	{
		std::string id;
		std::string category;
		std::string modelPath;
		core::Vector3 scale{1.0f, 1.0f, 1.0f};
		core::Vector3 position{0.0f, 0.0f, 0.0f};      // Transform情報（全Entity共通）
		core::Vector3 rotation{0.0f, 0.0f, 0.0f};      // Transform情報（全Entity共通）
		core::Vector3 colliderSize{0.0f, 0.0f, 0.0f}; // コライダーサイズ（0なら自動計算の予定だがまだ未完成）
		core::Vector3 colliderOffset{0.0f, 0.0f, 0.0f};

		std::unordered_map<std::string, float> floatProperties;
		std::unordered_map<std::string, std::string> stringProperties; // 例: {"idleAnim": "path/to/anim.mv1"}
	};
}