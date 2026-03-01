#include "ResourceManager.h"
#include <DxLib.h>
#include "core/utility/LogUtil.h"

namespace infrastructure
{
	int ResourceManager::loadModel(const std::string& filePath)
	{
		// キャッシュにある場合は重複しないように
		auto it = m_modelCache.find(filePath);
		if (it != m_modelCache.end()) return it->second;

		int handle = MV1LoadModel(filePath.c_str());
		if (handle == -1)
		{
			LOG_E("モデルの読み込みに失敗しました: %s", filePath.c_str());
			return -1;
		}

		m_modelCache[filePath] = handle;
		return handle;
	}

	void ResourceManager::unloadModel(const std::string& filePath)
	{
		auto it = m_modelCache.find(filePath);
		if (it == m_modelCache.end()) return;

		MV1DeleteModel(it->second);
		m_modelCache.erase(it);
	}

	void ResourceManager::unloadAll()
	{
		for (auto& [path, handle] : m_modelCache)
		{
			MV1DeleteModel(handle);
		}

		m_modelCache.clear();
	}
}