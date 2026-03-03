#pragma once
#include <unordered_map>
#include <string>
#include "core/interface/IResourceManager.h"

namespace infrastructure
{
	/**
	 * @brief リソースの読み込み・管理を担当するクラス
	 */
	class ResourceManager : public core::iface::IResourceManager
	{
	public:
		int loadModel(const std::string& filePath) override;
		void unloadModel(const std::string& filePath) override;
		void unloadAll() override;

	private:
		// ファイルパスをキーにしてモデルを管理
		std::unordered_map<std::string, int> m_modelCache;

	};
}