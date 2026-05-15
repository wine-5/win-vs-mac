#include "ResourceManager.h"
#include <stdexcept>

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
		catch (const std::exception&)
		{
			// リポジトリ初期化失敗時は、ポインタを null のままにする
			// 外側で処理が必要
		}
	}

	int ResourceManager::loadModelById(const std::string_view modelId)
	{
		return m_modelRepo->loadModelById(modelId);
	}

	std::optional<core::data::ModelMetadata> ResourceManager::getMetadata(const std::string_view modelId) const
	{
		return m_modelRepo->getMetadata(modelId);
	}

	std::optional<std::string> ResourceManager::getFontName(const std::string_view fontId) const
	{
		return m_fontRepo->getFontName(fontId);
	}

	int ResourceManager::getJobCount() const noexcept
	{
		return m_jobRepo->getJobCount();
	}

	core::iface::JobInfo ResourceManager::getJobInfo(core::constant::JobType jobType) const
	{
		return m_jobRepo->getJobInfo(jobType);
	}
}