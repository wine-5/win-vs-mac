#include "JobRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure
{
	JobRepository::JobRepository()
	{
		std::ifstream file("assets/data/jobData.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/jobData.json を開けませんでした");

		auto json = nlohmann::json::parse(file);
		const auto& jobs{ json["jobs"] };

		for (size_t i = 0; i < jobs.size() && i < m_jobTable.size(); ++i)
		{
			m_jobTable[i].m_id        = jobs[i]["id"];
			m_jobTable[i].m_name      = jobs[i]["name"];
			m_jobTable[i].m_skillName = jobs[i]["skillName"];
			m_jobTable[i].m_hp        = jobs[i]["hp"];
			m_jobTable[i].m_atk       = jobs[i]["atk"];
			m_jobTable[i].m_def       = jobs[i]["def"];
			m_jobTable[i].m_spd       = jobs[i]["spd"];
		}
	}

	int JobRepository::getJobCount() const noexcept
	{
		return core::constant::JOB_COUNT;
	}

	core::iface::JobInfo JobRepository::getJobInfo(core::constant::JobType jobType) const
	{
		return m_jobTable[static_cast<int>(jobType)];
	}
}