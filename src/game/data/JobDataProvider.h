#pragma once
#include <array>
#include "core/interface/IJobProvider.h"
#include "core/constant/JobType.h"

namespace game::data
{
	/**
	 * @brief 職業データプロバイダー
	 */
	class JobDataProvider : public core::iface::IJobProvider
	{
	public:
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

	private:
		/// @brief 職業データテーブル
		static const std::array<core::iface::JobInfo, core::constant::JOB_COUNT> JOB_TABLE;
	};
}