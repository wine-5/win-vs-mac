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

		int loadModelById(const std::string& modelId) override;
		std::optional<core::data::ModelMetadata> getMetadata(const std::string& modelId) const override;

	private:
		// TODO: オブジェクトの種類が増えるごとにloadXxxData()関数が肥大化するため、
		//       将来的にJsonLoaderクラスを作成し、汎用的なJSON読み込みシステムに移行する予定
		void loadPlayerData();
		void loadGroundData();
		core::data::ModelMetadata parseJsonFile(const std::string& filePath);

		// ファイルパスをキーにしてモデルを管理
		std::unordered_map<std::string, int> m_modelCache;
		std::unordered_map<std::string, int> m_modelHandles;
		std::unordered_map<std::string, core::data::ModelMetadata> m_metadata;

	public:
		virtual ~ResourceManager() = default;
	};
}