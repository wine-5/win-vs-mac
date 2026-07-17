#include "AnimationRepository.h"
#include <DxLib.h>
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace infrastructure::repository
{
	AnimationRepository::AnimationRepository()
	{
		std::ifstream file("assets/config/resources.json");
		if (!file.is_open())
			throw std::runtime_error("assets/config/resources.json を開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);
		if (!j.contains("animations")) return;

		for (const auto& item : j["animations"])
			m_paths[item["id"].get<std::string>()] = item["path"].get<std::string>();
	}

	AnimationRepository::~AnimationRepository()
	{
		for (auto& [id, handle] : m_handles)
			MV1DeleteModel(handle);
	}

	int AnimationRepository::loadAnimationById(std::string_view animationId)
	{
		const std::string id{ animationId };

		auto handleIt{ m_handles.find(id) };
		if (handleIt != m_handles.end())
			return handleIt->second;

		auto pathIt{ m_paths.find(id) };
		if (pathIt == m_paths.end())
		{
			LOG_E("アニメーションID '%s' が見つかりません", id.c_str());
			return -1;
		}

		const int handle{ MV1LoadModel(pathIt->second.c_str()) };
		if (handle == -1)
		{
			LOG_E("アニメーションの読み込みに失敗しました: %s", pathIt->second.c_str());
			return -1;
		}

		m_handles[id] = handle;
		return handle;
	}
} // namespace infrastructure::repository
