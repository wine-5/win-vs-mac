#include "ServiceLocatorInitializer.h"
#include "core/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "infrastructure/utility/LogUtil.h"

void ServiceLocatorInitializer::init()
{
	// ここにサービスロケータに登録するクラスを記載する

	// DEBUG:デバック用のためリリース時は消すこと
	core::ServiceLocator::provide<core::iface::ILogger>(
		std::make_unique<infrastructure::utility::LogUtil>()
	);
}