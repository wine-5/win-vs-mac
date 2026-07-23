#include "ImageRepository.h"
#include <DxLib.h>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"

namespace infrastructure::repository
{
	ImageRepository::ImageRepository(const nlohmann::json& j)
	{
        if (!j.contains("images")) return;

        for (const auto& item : j["images"])
            m_paths[item["id"].get<std::string>()] = item["path"].get<std::string>();
    }

    ImageRepository::~ImageRepository()
    {
        for (auto& [id, handle] : m_handles)
            DeleteGraph(handle);
    }

    int ImageRepository::loadImageById(std::string_view imageId)
    {
        const std::string id{ imageId };

        auto handleIt{ m_handles.find(id) };
        if (handleIt != m_handles.end())
            return handleIt->second;

        auto pathIt{ m_paths.find(id) };
        if (pathIt == m_paths.end())
        {
			core::log::error("画像ID '{}' が見つかりません", id.c_str());
			return -1;
        }

        const int handle{ LoadGraph(pathIt->second.c_str()) };
        if (handle == -1)
        {
			core::log::error("画像の読み込みに失敗しました: {}", pathIt->second.c_str());
			return -1;
        }

        m_handles[id] = handle;
        return handle;
    }
} // namespace infrastructure::repository
