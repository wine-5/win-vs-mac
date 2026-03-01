#include "ServiceLocatorInitializer.h"
#include "core/ServiceLocator.h"
#include "core/IResourceManager.h"
#include "infrastructure/Camera.h"
#include "infrastructure/Renderer.h"
#include "infrastructure/ResourceManager.h"

void ServiceLocatorInitializer::init()
{
	core::ServiceLocator::provide(std::make_unique<infrastructure::Camera>());
	core::ServiceLocator::provide(std::make_unique<infrastructure::Renderer>());
	core::ServiceLocator::provide<core::IResourceManager>(
		std::make_unique<infrastructure::ResourceManager>()
	);
}
