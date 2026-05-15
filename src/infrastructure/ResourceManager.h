#pragma once
#include <memory>
#include "core/interface/IResourceManager.h"
#include "core/interface/IJobProvider.h"
#include "infrastructure/repository/ModelRepository.h"
#include "infrastructure/repository/FontRepository.h"
#include "infrastructure/repository/JobRepository.h"

namespace infrastructure
{
	/// @brief Facade class that centralizes access to resources
	class ResourceManager : public core::iface::IResourceManager,
	                        public core::iface::IJobProvider
	{
	public:
		ResourceManager();
		~ResourceManager() = default;

		// IResourceManager
		int loadModelById(const std::string_view modelId) override;
		std::optional<core::data::ModelMetadata> getMetadata(const std::string_view modelId) const override;
		std::optional<std::string> getFontName(const std::string_view fontId) const override;

		// IJobProvider
		int getJobCount() const noexcept override;
		core::iface::JobInfo getJobInfo(core::constant::JobType jobType) const override;

	private:
		std::unique_ptr<ModelRepository> m_modelRepo;
		std::unique_ptr<FontRepository>  m_fontRepo;
		std::unique_ptr<JobRepository>   m_jobRepo;
	};
}