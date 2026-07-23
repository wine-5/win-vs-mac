#pragma once
#include "core/data/StageMetadata.h"

namespace infrastructure::repository
{
	/**
	 * @brief ステージ配置定義（stageData.json）の読み込みを担当
	 */
	class StageRepository
	{
	public:
		/**
		 * @brief StageRepositoryのコンストラクタ
		 *
		 * stageData.json を読み込み、配置定義を構築する
		 */
		StageRepository();

		/**
		 * @brief ステージの配置定義を取得する
		 * @return ステージ配置定義
		 */
		[[nodiscard]] const core::data::StageMetadata& getStageMetadata() const noexcept;

	private:
		core::data::StageMetadata m_stageMetadata{};
	};
} // namespace infrastructure::repository