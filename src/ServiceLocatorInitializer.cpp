#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "core/interface/IResourceManager.h"
#include "platform/WindowsDataProvider.h"
#include "infrastructure/Screen.h"
#include "infrastructure/utility/LogUtil.h"
#include "infrastructure/ResourceManager.h"
#include "game/scene/SceneManager.h"
#include "game/GameManager.h"
#include "core/interface/IJobProvider.h"

void ServiceLocatorInitializer::init(int screenWidth, int screenHeight)
{
	core::base::ServiceLocator::provide<core::iface::IFileProvider>(
		std::make_unique<platform::WindowsDataProvider>()
	);

	// ResourceManagerをIResourceManagerインターフェースで登録（所有権を持たない）
	core::base::ServiceLocator::provideExisting<core::iface::IResourceManager>(
		&infrastructure::ResourceManager::getInstance()
	);

	// ResourceManagerをIJobProviderインターフェースでも登録（所有権を持たない）
	core::base::ServiceLocator::provideExisting<core::iface::IJobProvider>(
		&infrastructure::ResourceManager::getInstance()
	);

	// DEBUG:デバック用のためリリース時は消すこと
	core::base::ServiceLocator::provide<core::iface::ILogger>(
		std::make_unique<infrastructure::utility::LogUtil>()
	);
	// Screen登録（SetGraphMode()で設定した画面サイズを渡す）
	core::base::ServiceLocator::provide<core::iface::IScreen>(
		std::make_unique<infrastructure::Screen>(screenWidth, screenHeight)
	);

	// SceneManager登録（内部でSceneFactoryを所有）
	core::base::ServiceLocator::provide(
		std::make_unique<game::scene::SceneManager>()
	);

}