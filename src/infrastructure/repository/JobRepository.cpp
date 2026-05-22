#include "JobRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"

namespace infrastructure
{
	JobRepository::JobRepository()
	{
		std::ifstream file("assets/data/jobData.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/jobData.json を開けませんでした");

		auto json = nlohmann::json::parse(file);
		const auto& jobs{ json["jobs"] };

		auto* stringConverter = core::base::ServiceLocator::get<core::iface::IStringConverter>();

		for (size_t i = 0; i < jobs.size() && i < m_jobTable.size(); ++i)
		{
			std::string nameUtf8 = jobs[i]["name"];
			std::string skillNameUtf8 = jobs[i]["skillName"];

			m_jobTable[i].m_id        = jobs[i]["id"];
			m_jobTable[i].m_name      = stringConverter->utf8ToShiftJis(nameUtf8);
			m_jobTable[i].m_skillName = stringConverter->utf8ToShiftJis(skillNameUtf8);
			m_jobTable[i].m_hp        = jobs[i]["hp"];
			m_jobTable[i].m_atk       = jobs[i]["atk"];
			m_jobTable[i].m_def       = jobs[i]["def"];
			m_jobTable[i].m_spd       = jobs[i]["spd"];
		}
	}

	core::data::JobInfo JobRepository::getJobInfo(core::constant::JobType jobType) const
	{
		return m_jobTable[static_cast<int>(jobType)];
	}
}