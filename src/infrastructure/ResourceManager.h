#pragma once
#include <unordered_map>
#include <vector>
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
		/**
		 * @brief ResourceManagerのコンストラクタ
		 */
		ResourceManager();

		/**
		 * @brief modelIDからモデルを読み込み、ハンドルを返す
		 * @param modelId モデルID
		 * @return モデルハンドル
		 */
		int loadModelById(const std::string& modelId) override;
		
		/**
		 * @brief modelIDからメタデータを取得する
		 * @param modelId モデルID
		 * @return メタデータ（存在しない場合nullopt）
		 */
		std::optional<core::data::ModelMetadata> getMetadata(const std::string& modelId) const override;

	private:
		/// @brief リソース定義（resources.jsonの1エントリ）
		struct ResourceDefinition {
			std::string id;
			std::string path;
		};

		/// @brief resources.jsonからリソースリストを読み込む
		std::vector<ResourceDefinition> loadResourceList(const std::string& filePath);
		core::data::ModelMetadata parseJsonFile(const std::string& filePath);

		// ファイルパスをキーにしてモデルを管理
		std::unordered_map<std::string, int> m_modelCache;
		std::unordered_map<std::string, int> m_modelHandles;
		std::unordered_map<std::string, core::data::ModelMetadata> m_metadata;

	public:
		virtual ~ResourceManager() = default;
	};
}