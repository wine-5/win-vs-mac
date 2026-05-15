#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <vector>
#include "core/data/ModelMetadata.h"
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure
{
	class ModelRepository
	{
	public:
		ModelRepository();

		int loadModelById(std::string_view modelId);
		std::optional<core::data::ModelMetadata> getMetadata(std::string_view modelId) const;

	private:
		struct ResourceDefinition
		{
			std::string m_id;
			std::string m_path;
		};

		std::vector<ResourceDefinition> loadResourceList(const nlohmann::json& json);
		core::data::ModelMetadata parseJsonFile(const std::string& filePath);

		std::unordered_map<std::string, int> m_modelHandles;
		std::unordered_map<std::string, core::data::ModelMetadata> m_metadata;
	};
}
