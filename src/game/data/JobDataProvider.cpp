#include "JobDataProvider.h"

namespace game::data
{
	const std::array<core::iface::JobInfo, core::constant::JOB_COUNT> JobDataProvider::JOB_TABLE = {
		core::iface::JobInfo{ 0, "剣士", "全方位斬り", 120.0f, 45.0f, 35.0f, 30.0f },
		core::iface::JobInfo{ 1, "魔法使い", "巨大魔法弾", 80.0f, 60.0f, 20.0f, 40.0f },
		core::iface::JobInfo{ 2, "忍者", "分身一斉攻撃", 90.0f, 50.0f, 25.0f, 55.0f }
	};

	int JobDataProvider::getJobCount() const noexcept
	{
		return core::constant::JOB_COUNT;
	}

	core::iface::JobInfo JobDataProvider::getJobInfo(core::constant::JobType jobType) const
	{
		return JOB_TABLE[static_cast<int>(jobType)];
	}
}