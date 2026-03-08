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
		ResourceManager();

		int loadModel(const std::string& filePath) override;
		int loadModelById(const std::string& modelId) override;
		std::optional<core::data::ModelMetadata> getMetadata(const std::string& modelId) const override;

	private:
		void loadPlayerData();
		core::data::ModelMetadata parseJsonFile(const std::string& filePath);

		// ファイルパスをキーにしてモデルを管理
		std::unordered_map<std::string, int> m_modelCache;
		std::unordered_map<std::string, int> m_modelHandles;
		std::unordered_map<std::string, core::data::ModelMetadata> m_metadata;

	public:
		virtual ~ResourceManager() = default;
	};
}