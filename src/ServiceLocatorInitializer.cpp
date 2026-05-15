#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IStringConverter.h"
#include "platform/WindowsDataProvider.h"
#include "platform/utility/StringConverter.h"
#include "infrastructure/Screen.h"
#include "infrastructure/utility/LogUtil.h"
#include "infrastructure/ResourceManager.h"
#include "game/scene/SceneManager.h"
#include "game/GameManager.h"
#include "core/interface/IJobProvider.h"

void ServiceLocatorInitializer::init(int screenWidth, int screenHeight)
{
	// ファイルプロバイダを登録
	core::base::ServiceLocator::provide<core::iface::IFileProvider>(
		std::make_unique<platform::WindowsDataProvider>()
	);

	// 文字列変換プロバイダを登録
	core::base::ServiceLocator::provide<core::iface::IStringConverter>(
		std::make_unique<platform::utility::StringConverter>()
	);

	// ResourceManager を生成して登録（Facade パターン：内部でリポジトリが管理）
	try
	{
		auto resourceManager = std::make_unique<infrastructure::ResourceManager>();
		auto* resourceManagerPtr = resourceManager.get();

		core::base::ServiceLocator::provide<core::iface::IResourceManager>(
			std::move(resourceManager)
		);

		// ResourceManager を IJobProvider インターフェースでも登録
		core::base::ServiceLocator::provideExisting<core::iface::IJobProvider>(
			resourceManagerPtr
		);
	}
	catch (const std::exception& e)
	{
		LOG_E("ResourceManager の初期化に失敗しました: %s", e.what());
		return;
	}

	// デバッグ用ロガーを登録
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