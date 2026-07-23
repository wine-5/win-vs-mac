#include "ResourceManager.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace
{
	constexpr const char* RESOURCE_CONFIG_PATH{ "assets/config/resources.json" };
} // namespace

namespace infrastructure::resource
{
	ResourceManager::ResourceManager()
	{
		// resources.json は複数のリポジトリが参照するため、ここで1回だけ読んで共有する
		std::ifstream file{ RESOURCE_CONFIG_PATH };
		if (!file.is_open())
			throw std::runtime_error{ std::string{ RESOURCE_CONFIG_PATH } + " を開けませんでした" };

		const nlohmann::json config{ nlohmann::json::parse(file) };

		// リソースが揃わないまま起動すると「モデルが出ない」等の中途半端な状態で
		// 原因究明が遅れるため、例外は握りつぶさず呼び出し元へ伝播させる（Fail Fast）
		m_modelRepo = std::make_unique<repository::ModelRepository>(config);
		m_fontRepo = std::make_unique<repository::FontRepository>(config);
		m_imageRepo = std::make_unique<repository::ImageRepository>(config);
		m_animRepo = std::make_unique<repository::AnimationRepository>(config);
		m_stageRepo = std::make_unique<repository::StageRepository>();
		m_projectileRepo = std::make_unique<repository::ProjectileRepository>();
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

	int ResourceManager::loadImageById(std::string_view imageId)
	{
		return m_imageRepo->loadImageById(imageId);
	}

	int ResourceManager::loadAnimationById(std::string_view animationId)
	{
		return m_animRepo->loadAnimationById(animationId);
	}

	const core::data::StageMetadata& ResourceManager::getStageMetadata() const noexcept
	{
		return m_stageRepo->getStageMetadata();
	}

	const core::data::ProjectileMetadata& ResourceManager::getProjectileMetadata(std::string_view projectileId) const
	{
		return m_projectileRepo->getMetadata(projectileId);
	}

	int ResourceManager::duplicateModel(int modelHandle)
	{
		return m_modelRepo->duplicateModel(modelHandle);
	}

	void ResourceManager::detachAllAnimations(int modelHandle)
	{
		m_modelRepo->detachAllAnimations(modelHandle);
	}

	float ResourceManager::computeBoundingRadius(int modelHandle, float scale) const noexcept
	{
		return m_modelRepo->computeBoundingRadius(modelHandle, scale);
	}

	core::Vector3 ResourceManager::computeBoundingCenter(int modelHandle) const noexcept
	{
		return m_modelRepo->computeBoundingCenter(modelHandle);
	}
} // namespace infrastructure::resource