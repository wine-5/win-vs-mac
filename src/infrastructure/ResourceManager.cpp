#include "ResourceManager.h"
#include <stdexcept>
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	ResourceManager::ResourceManager()
	{
		try
		{
			m_modelRepo = std::make_unique<ModelRepository>();
			m_fontRepo = std::make_unique<FontRepository>();
			m_jobRepo = std::make_unique<JobRepository>();
		}
		catch (const std::exception& e)
		{
			LOG_E("リポジトリ初期化に失敗しました: %s", e.what());
		}
	}

	int ResourceManager::loadModelById(const std::string_view modelId)
	{
		if (!m_modelRepo)
			return -1;
		return m_modelRepo->loadModelById(modelId);
	}

	std::optional<core::data::ModelMetadata> ResourceManager::getMetadata(const std::string_view modelId) const
	{
		if (!m_modelRepo)
			return std::nullopt;
		return m_modelRepo->getMetadata(modelId);
	}

	std::optional<std::string> ResourceManager::getFontName(const std::string_view fontId) const
	{
		if (!m_fontRepo)
			return std::nullopt;
		return m_fontRepo->getFontName(fontId);
	}

	int ResourceManager::getJobCount() const noexcept
	{
		if (!m_jobRepo)
			return 0;
		return m_jobRepo->getJobCount();
	}

	core::iface::JobInfo ResourceManager::getJobInfo(core::constant::JobType jobType) const
	{
		if (!m_jobRepo)
			throw std::runtime_error("JobRepository が初期化されていません");
		return m_jobRepo->getJobInfo(jobType);
	}
}