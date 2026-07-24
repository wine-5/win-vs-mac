#pragma once
#include <memory>
#include "core/interface/IResourceManager.h"
#include "infrastructure/resource/repository/ModelRepository.h"
#include "infrastructure/resource/repository/FontRepository.h"
#include "infrastructure/resource/repository/ImageRepository.h"
#include "infrastructure/resource/repository/AnimationRepository.h"
#include "infrastructure/resource/repository/StageRepository.h"
#include "infrastructure/resource/repository/StageCatalogRepository.h"
#include "infrastructure/resource/repository/ExtensionBonusRepository.h"
#include "infrastructure/resource/repository/ProjectileRepository.h"

namespace infrastructure::resource
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
		 * @brief ファイルパスから直接モデルを読み込み、ハンドルを返す（キャッシュ付き）
		 * @param path モデルファイルのパス
		 * @return モデルハンドル、失敗時は-1
		 */
		int loadModelByPath(std::string_view path) override;

		/**
		 * @brief modelIDからメタデータを取得する
		 * @param modelId モデルID
		 * @return メタデータ（存在しない場合nullopt）
		 */
		[[nodiscard]] std::optional<core::data::ModelMetadata> getMetadata(const std::string_view modelId) const override;

		/**
		 * @brief フォントIDからフォント名を取得する
		 * @param fontId フォントID
		 * @return フォントファミリー名（存在しない場合nullopt）
		 */
		[[nodiscard]] std::optional<std::string> getFontName(const std::string_view fontId) const override;

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
		[[nodiscard]] const core::data::StageMetadata& getStageMetadata() const noexcept override;

		/**
		 * @brief 配置物の種類IDから種類定義を取得する
		 * @param type 種類ID（stageCatalog.jsonのprops[].id）
		 * @return 配置物の種類定義（存在しない場合はthrow）
		 */
		[[nodiscard]] const core::data::PropDefinition& getPropDefinition(std::string_view type) const override;

		/**
		 * @brief 拡張子種別に対応するパラメータボーナスを取得する
		 * @param type ファイル拡張子グループ種別
		 * @return 対応するボーナス値
		 */
		[[nodiscard]] const core::data::FileExtensionBonus& getExtensionBonus(
		    core::data::FileExtensionType type) const noexcept override;

		/**
		 * @brief 弾IDから弾定義を取得する
		 * @param projectileId 弾ID（projectileData.json で定義）
		 * @return 弾定義
		 */
		[[nodiscard]] const core::data::ProjectileMetadata& getProjectileMetadata(std::string_view projectileId) const override;

		/**
		 * @brief モデルハンドルを複製する
		 * @param modelHandle 複製元のモデルハンドル
		 * @return 複製したモデルハンドル、失敗時は-1
		 */
		[[nodiscard]] int duplicateModel(int modelHandle) override;

		/**
		 * @brief モデルにアタッチされている全アニメーションをデタッチする
		 * @param modelHandle 対象のモデルハンドル
		 */
		void detachAllAnimations(int modelHandle) override;

		/**
		 * @brief モデルの水平方向の外接半径を計算する（弾などの当たり判定サイズ自動取得用）
		 * @param modelHandle モデルハンドル
		 * @param scale 適用するスケール
		 * @return 水平方向の外接半径。失敗時は 0.0f
		 */
		[[nodiscard]] float computeBoundingRadius(int modelHandle, float scale) const noexcept override;

		[[nodiscard]] core::Vector3 computeBoundingCenter(int modelHandle) const noexcept override;

	  private:
		std::unique_ptr<repository::ModelRepository> m_modelRepo;
		std::unique_ptr<repository::FontRepository> m_fontRepo;
		std::unique_ptr<repository::ImageRepository> m_imageRepo;
		std::unique_ptr<repository::AnimationRepository> m_animRepo;
		std::unique_ptr<repository::StageRepository> m_stageRepo;
		std::unique_ptr<repository::StageCatalogRepository> m_stageCatalogRepo;
		std::unique_ptr<repository::ExtensionBonusRepository> m_extensionBonusRepo;
		std::unique_ptr<repository::ProjectileRepository> m_projectileRepo;
	};
} // namespace infrastructure::resource