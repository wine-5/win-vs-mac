#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "platform/WindowsDataProvider.h"
#include "infrastructure/Screen.h"
#include "infrastructure/utility/LogUtil.h"
#include "game/scene/SceneManager.h"
#include "game/GameManager.h"
#include "core/interface/IResourceManager.h"
#include "infrastructure/ResourceManager.h"
#include "core/interface/IJobProvider.h"
#include "game/data/JobDataProvider.h"

void ServiceLocatorInitializer::init(int screenWidth, int screenHeight)
{
	core::base::ServiceLocator::provide(
		std::make_unique<game::GameManager>()
	);

	core::base::ServiceLocator::provide<core::iface::IFileProvider>(
		std::make_unique<platform::WindowsDataProvider>()
	);

	// ResourceManagerを登録（モデル・メタデータ・フォント管理用）
	core::base::ServiceLocator::provide<core::iface::IResourceManager>(
		std::make_unique<infrastructure::ResourceManager>()
	);

	// JobDataProviderを登録（職業情報提供用）
	core::base::ServiceLocator::provide<core::iface::IJobProvider>(
		std::make_unique<game::data::JobDataProvider>()
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