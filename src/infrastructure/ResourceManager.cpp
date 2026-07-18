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
			m_modelRepo = std::make_unique<repository::ModelRepository>();
			m_fontRepo = std::make_unique<repository::FontRepository>();
			m_jobRepo = std::make_unique<repository::JobRepository>();
			m_imageRepo = std::make_unique<repository::ImageRepository>();
			m_animRepo = std::make_unique<repository::AnimationRepository>();
			m_stageRepo = std::make_unique<repository::StageRepository>();
			m_projectileRepo = std::make_unique<repository::ProjectileRepository>();
		}
		catch (const std::exception& e)
		{
			LOG_E("リポジトリ初期化に失敗しました: {}", e.what());
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

	core::data::JobInfo ResourceManager::getJobInfo(core::constant::JobType jobType) const
	{
		if (!m_jobRepo)
			throw std::runtime_error("JobRepository が初期化されていません");
		return m_jobRepo->getJobInfo(jobType);
	}

	int ResourceManager::loadImageById(std::string_view imageId)
	{
		if (!m_imageRepo)
			return -1;
		return m_imageRepo->loadImageById(imageId);
	}

	int ResourceManager::loadAnimationById(std::string_view animationId)
	{
		if (!m_animRepo)
			return -1;
		return m_animRepo->loadAnimationById(animationId);
	}

	const core::data::StageMetadata& ResourceManager::getStageMetadata() const
	{
		if (!m_stageRepo)
			throw std::runtime_error("StageRepository が初期化されていません");
		return m_stageRepo->getStageMetadata();
	}

	const core::data::ProjectileMetadata& ResourceManager::getProjectileMetadata(std::string_view projectileId) const
	{
		if (!m_projectileRepo)
			throw std::runtime_error("ProjectileRepository が初期化されていません");
		return m_projectileRepo->getMetadata(projectileId);
	}

	int ResourceManager::duplicateModel(int modelHandle)
	{
		if (!m_modelRepo)
			return -1;
		return m_modelRepo->duplicateModel(modelHandle);
	}

	float ResourceManager::computeBoundingRadius(int modelHandle, float scale) const
	{
		if (!m_modelRepo)
			return 0.0f;
		return m_modelRepo->computeBoundingRadius(modelHandle, scale);
	}
} // namespace infrastructure