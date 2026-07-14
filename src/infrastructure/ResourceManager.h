#pragma once
#include <memory>
#include "core/interface/IResourceManager.h"
#include "infrastructure/repository/ModelRepository.h"
#include "infrastructure/repository/FontRepository.h"
#include "infrastructure/repository/JobRepository.h"
#include "infrastructure/repository/ImageRepository.h"
#include "infrastructure/repository/AnimationRepository.h"
#include "infrastructure/repository/StageRepository.h"

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

		/**
		 * @brief 画像IDから画像を読み込みハンドルを返す（キャッシュ付き）
		 * @param imageId 画像ID
		 * @return DxLib 画像ハンドル、失敗時は -1
		 */
		int loadImageById(std::string_view imageId) override;

		/**
		 * @brief アニメーションIDからアニメーションモデルを読み込みハンドルを返す（キャッシュ付き）
		 * @param animationId アニメーションID
		 * @return DxLib モデルハンドル、失敗時は -1
		 */
		int loadAnimationById(std::string_view animationId) override;

		/**
		 * @brief ステージの配置定義を取得する
		 * @return ステージ配置定義
		 */
		[[nodiscard]] const core::data::StageMetadata& getStageMetadata() const override;

		/**
		 * @brief モデルハンドルを複製する
		 * @param modelHandle 複製元のモデルハンドル
		 * @return 複製したモデルハンドル、失敗時は-1
		 */
		int duplicateModel(int modelHandle) override;

	private:
		std::unique_ptr<ModelRepository>     m_modelRepo;
		std::unique_ptr<FontRepository>      m_fontRepo;
		std::unique_ptr<JobRepository>       m_jobRepo;
		std::unique_ptr<ImageRepository>     m_imageRepo;
		std::unique_ptr<AnimationRepository> m_animRepo;
		std::unique_ptr<StageRepository>     m_stageRepo;
	};
}