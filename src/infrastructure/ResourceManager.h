#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "core/interface/IResourceManager.h"
#include "core/interface/IJobProvider.h"
#include "core/constant/JobType.h"
#include "core/base/Singleton.h"
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure
{
	/**
	 * @brief リソースの読み込み・管理を担当するクラス
	 * 現在ResourceManagerが肥大化かつ責務が複数あるため
	 * 将来的に分離してResourceManagerがFacadeの役割を果たすように設計しなおす
	 */
	class ResourceManager : public core::iface::IResourceManager,
	                        public core::iface::IJobProvider,
	                        public core::base::Singleton<ResourceManager>
	{
	public:
		friend class core::base::Singleton<ResourceManager>;

		/**
		 * @brief modelIDからモデルを読み込み、ハンドルを返す
		 * @param modelId モデルID
		 * @return モデルハンドル
		 */
		int loadModelById(const std::string_view modelId) override;

		/**
		 * @brief modelIDからメタデータを取得する
		 * @param modelId モデルID
		 * @return メタデータ（存在しない場合nullopt）
		 */
		std::optional<core::data::ModelMetadata> getMetadata(const std::string_view modelId) const override;

		/**
		 * @brief フォントIDからフォント名を取得する
		 * @param fontId フォントID
		 * @return フォントファミリー名（存在しない場合nullopt）
		 */
		std::optional<std::string> getFontName(const std::string_view fontId) const override;

		/**
		* @brief 職業数を取得
		* @return 職業数（3）
		*/
		int getJobCount() const noexcept override;

		/**
	  * @brief 職業情報を取得
	  * @param jobType 職業タイプ
	  * @return 職業情報
	  */
		core::iface::JobInfo getJobInfo(core::constant::JobType jobType) const override;

		~ResourceManager();

	private:

		/**
		 * @brief ResourceManagerのコンストラクタ
		 */
		ResourceManager();
		/// @brief リソース定義（resources.jsonの1エントリ）
		struct ResourceDefinition
		{
			std::string m_id;
			std::string m_path;
		};

		struct FontDataDefinition
		{
			std::string m_id;
			std::string m_path;
			std::string m_name;
		};

		/// @brief resources.jsonからリソースリストを読み込む
		std::vector<ResourceDefinition> loadResourceList(const nlohmann::json& json);
		/// @brief JSONファイルからモデルメタデータを読み込む
		core::data::ModelMetadata parseJsonFile(const std::string& filePath);
		/// @brief resources.jsonからフォントリストを読み込む
		std::vector<FontDataDefinition> loadFontList(const nlohmann::json& json);
		/// @brief jobData.jsonから職業データを読み込む
		void loadJobsFromJson();
		/// @brief ファイルが開けない場合、ログ・アサート・例外を発生させる
		void throwIfFileNotOpen(const std::ifstream& file, const std::string& filePath);

		// ファイルパスをキーにしてモデルを管理
		std::unordered_map<std::string, int> m_modelCache;
		std::unordered_map<std::string, int> m_modelHandles;
		std::unordered_map<std::string, core::data::ModelMetadata> m_metadata;
		std::unordered_map<std::string, std::string> m_fontNames;  // fontId -> fontFamilyName
		std::unordered_map<std::string, std::string> m_fontPaths;  // fontId -> filePath（解放用）
		std::array<core::iface::JobInfo, core::constant::JOB_COUNT> m_jobTable;
	};
}