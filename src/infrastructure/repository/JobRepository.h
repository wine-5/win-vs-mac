#pragma once
#include <array>
#include "core/data/JobInfo.h"
#include "core/constant/JobType.h"

namespace infrastructure
{
	/**
	 * @brief ジョブ/職業データを管理するリポジトリクラス
	 *
	 * jobData.jsonからジョブデータを読み込み、
	 * JobType列挙値でジョブ情報を取得する
	 */
	class JobRepository
	{
	public:
		/**
		 * @brief コンストラクタ
		 *
		 * コンストラクト時にjobData.jsonからすべてのジョブデータを読み込む
		 * @throw std::runtime_error ファイルが見つからないか、JSONパースに失敗した場合
		 */
		JobRepository();

		/**
		 * @brief ジョブタイプでジョブ情報を取得する
		 *
		 * @param jobType ジョブタイプ列挙値
		 * @return ジョブ情報（ステータス、名前、スキルなど）
		 */
		core::data::JobInfo getJobInfo(core::constant::JobType jobType) const;

	private:
		std::array<core::data::JobInfo, core::constant::JOB_COUNT> m_jobTable;
	};
} // namespace infrastructure