#include "ImageRepository.h"
#include <DxLib.h>
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
    ImageRepository::ImageRepository()
    {
        std::ifstream file("assets/config/resources.json");
        if (!file.is_open())
            throw std::runtime_error("assets/config/resources.json を開けませんでした");

        const nlohmann::json j = nlohmann::json::parse(file);
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
            LOG_E("画像ID '%s' が見つかりません", id.c_str());
            return -1;
        }

        const int handle{ LoadGraph(pathIt->second.c_str()) };
        if (handle == -1)
        {
            LOG_E("画像の読み込みに失敗しました: %s", pathIt->second.c_str());
            return -1;
        }

        m_handles[id] = handle;
        return handle;
    }
} // namespace infrastructure
