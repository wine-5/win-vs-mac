#pragma once
#include <array>
#include "core/interface/IJobProvider.h"
#include "core/constant/JobType.h"

namespace infrastructure
{
	class JobRepository
	{
	public:
		JobRepository();

		int getJobCount() const noexcept;
		core::iface::JobInfo getJobInfo(core::constant::JobType jobType) const;

	private:
		std::array<core::iface::JobInfo, core::constant::JOB_COUNT> m_jobTable;
	};
}
