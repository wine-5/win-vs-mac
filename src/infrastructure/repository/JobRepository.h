#pragma once
#include <array>
#include "core/interface/IJobProvider.h"
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
		 * @brief 利用可能なジョブの総数を取得する
		 *
		 * @return JOB_COUNTで定義されたジョブ数
		 */
		int getJobCount() const noexcept;

		/**
		 * @brief ジョブタイプでジョブ情報を取得する
		 *
		 * @param jobType ジョブタイプ列挙値
		 * @return ジョブ情報（ステータス、名前、スキルなど）
		 */
		core::iface::JobInfo getJobInfo(core::constant::JobType jobType) const;

	private:
		std::array<core::iface::JobInfo, core::constant::JOB_COUNT> m_jobTable;
	};
}