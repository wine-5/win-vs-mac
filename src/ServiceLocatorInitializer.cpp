#include "ServiceLocatorInitializer.h"
#include "core/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "infrastructure/Screen.h"
#include "core/interface/IScreen.h"
#include "infrastructure/utility/LogUtil.h"

void ServiceLocatorInitializer::init()
{
	// Screen登録（コンストラクタで画面サイズを取得する）
	core::ServiceLocator::provide<core::iface::IScreen>(
		std::make_unique<infrastructure::Screen>()
	);
	
	// DEBUG:デバック用のためリリース時は消すこと
	core::ServiceLocator::provide<core::iface::ILogger>(
		std::make_unique<infrastructure::utility::LogUtil>()
	);
}