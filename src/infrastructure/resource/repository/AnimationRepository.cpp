#include "AnimationRepository.h"
#include <DxLib.h>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"

namespace infrastructure::resource::repository
{
	AnimationRepository::AnimationRepository(const nlohmann::json& j)
	{
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
			core::log::error("アニメーションID '{}' が見つかりません", id.c_str());
			return -1;
		}

		const int handle{ MV1LoadModel(pathIt->second.c_str()) };
		if (handle == -1)
		{
			core::log::error("アニメーションの読み込みに失敗しました: {}", pathIt->second.c_str());
			return -1;
		}

		m_handles[id] = handle;
		return handle;
	}
} // namespace infrastructure::resource::repository
