#include "ServiceLocatorInitializer.h"
#include "core/ServiceLocator.h"

void ServiceLocatorInitializer::init()
{
	// ここにサービスロケータに登録するクラスを記載する
	//例）
	//core::ServiceLocator::provide(std::make_unique<infrastructure::Camera>());
}
