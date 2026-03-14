#include "ServiceLocatorInitializer.h"
#include "core/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "infrastructure/Screen.h"
#include "infrastructure/utility/LogUtil.h"
#include "game/scene/SceneManager.h"

void ServiceLocatorInitializer::init(int screenWidth, int screenHeight)
{
	// DEBUG:デバック用のためリリース時は消すこと
	core::ServiceLocator::provide<core::iface::ILogger>(
		std::make_unique<infrastructure::utility::LogUtil>()
	);
	// Screen登録（SetGraphMode()で設定した画面サイズを渡す）
	core::ServiceLocator::provide<core::iface::IScreen>(
		std::make_unique<infrastructure::Screen>(screenWidth, screenHeight)
	);
	
	// SceneManager登録（内部でSceneFactoryを所有）
	core::ServiceLocator::provide(
		std::make_unique<game::scene::SceneManager>()
	);
	
}