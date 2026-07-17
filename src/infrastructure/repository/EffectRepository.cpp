#include "EffectRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/effekseer/EffekseerForDXLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure::repository
{
	EffectRepository::~EffectRepository()
	{
		for (const auto& [type, config] : m_configs)
		{
			if (config.m_handle != -1)
				DeleteEffekseerEffect(config.m_handle);
		}
	}

	void EffectRepository::initialize()
	{
		std::ifstream file{ "assets/config/resources.json" };
		if (!file.is_open())
			throw std::runtime_error{ "resoures.jsonを開けませんでした" };

		nlohmann::json json{};
		file >> json;

		load(json);
	}

	const std::unordered_map<core::constant::EffectType, EffectConfig>& EffectRepository::getAllConfigs() const
	{
		return m_configs;
	}

	void EffectRepository::load(const nlohmann::json& json)
	{
		if (!json.contains("effects")) return;

		const std::unordered_map<std::string, core::constant::EffectType> typeMap
		{
			{ "Hit", core::constant::EffectType::Hit },
		};

		for (const auto& entry : json["effects"])
		{
			const std::string key  { entry["type"] };
			const std::string path { entry["path"] };

			auto it{ typeMap.find(key) };
			if (it == typeMap.end()) continue;

			int handle{ LoadEffekseerEffect(path.c_str()) };
			if (handle == -1) continue;

			if (!entry.contains("poolSize") || !entry.contains("yOffset") || !entry.contains("scale"))
				throw std::runtime_error{ "effect '" + key + "' に必須フィールド (poolSize / yOffset / scale) が設定されていません" };

			EffectConfig config{};
			config.m_handle   = handle;
			config.m_poolSize = entry["poolSize"].get<int>();
			config.m_yOffset  = entry["yOffset"].get<float>();
			config.m_scale    = entry["scale"].get<float>();

			m_configs[it->second] = config;
		}
	}
} // namespace infrastructure::repository