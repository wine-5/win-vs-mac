#include "EffectRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/effekseer/EffekseerForDXLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	EffectRepository::EffectRepository()
	{
		std::ifstream file{ "assets/config/resources.json" };
		if (!file.is_open())
			throw std::runtime_error{ "resoures.jsonを開けませんでした" };

		nlohmann::json json{};
		file >> json;

		load(json);
	}

	int EffectRepository::getHandle(core::constant::EffectType type) const
	{
		auto it{m_handles.find(type)};
		if (it == m_handles.end()) return -1;
		return it->second;
	}

	void EffectRepository::load(const nlohmann::json& json)
	{
		if (!json.contains("effects")) return;

		const std::unordered_map<std::string, core::constant::EffectType> typeMap
		{
			{ "Hit",     core::constant::EffectType::Hit },
		};

		for (const auto& entry : json["effects"])
		{
			const std::string key  {entry["type"]};
			const std::string path {entry["path"]};

			auto it{ typeMap.find(key) };
			if (it == typeMap.end()) continue;

			int handle{ LoadEffekseerEffect(path.c_str()) };
			if (handle == - 1)
			{
				LOG("エフェクトの読み込みに失敗しました");
				continue;
			}

			m_handles[it->second] = handle;
		}
	}
}