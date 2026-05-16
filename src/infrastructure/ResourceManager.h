#pragma once
#include <memory>
#include "core/interface/IResourceManager.h"
#include "infrastructure/repository/ModelRepository.h"
#include "infrastructure/repository/FontRepository.h"
#include "infrastructure/repository/JobRepository.h"

namespace infrastructure
{
	/**
	 * @brief リソース管理の Facade クラス
	 * 各リポジトリ（Model・Font・Job）への一元アクセスを提供する
	 */
	class ResourceManager : public core::iface::IResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager() = default;

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
		 * @brief ジョブタイプからジョブ情報を取得する
		 * @param jobType ジョブタイプ
		 * @return ジョブ情報
		 */
		[[nodiscard]] core::data::JobInfo getJobInfo(core::constant::JobType jobType) const override;

	private:
		std::unique_ptr<ModelRepository> m_modelRepo;
		std::unique_ptr<FontRepository>  m_fontRepo;
		std::unique_ptr<JobRepository>   m_jobRepo;
	};
}