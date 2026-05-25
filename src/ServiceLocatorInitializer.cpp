#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IStringConverter.h"
#include "core/interface/IWindowFactory.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IAnimator.h"
#include "platform/WindowsDataProvider.h"
#include "platform/utility/StringConverter.h"
#include "platform/window/WindowFactory.h"
#include "infrastructure/Screen.h"
#include "infrastructure/utility/LogUtil.h"
#include "infrastructure/ResourceManager.h"
#include "infrastructure/UIRenderer.h"
#include "infrastructure/InputManager.h"
#include "infrastructure/Camera.h"
#include "infrastructure/Renderer.h"
#include "infrastructure/Animator.h"
#include "game/scene/SceneManager.h"
#include "core/interface/IPerformanceDataProvider.h"
#include "platform/system/WindowsPerformanceProvider.h"
#include "core/interface/IEffectFactory.h"
#include "infrastructure/EffectFactory.h"

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
	auto screen = std::make_unique<infrastructure::Screen>(screenWidth, screenHeight);
	auto* screenPtr = screen.get();
	core::base::ServiceLocator::provide<core::iface::IScreen>(
		std::move(screen)
	);

	// UIRenderer登録
	core::base::ServiceLocator::provide<core::iface::IUIRenderer>(
		std::make_unique<infrastructure::UIRenderer>()
	);

	// InputManager登録
	core::base::ServiceLocator::provide<core::iface::IInputProvider>(
		std::make_unique<infrastructure::InputManager>()
	);

	// Camera登録
	core::base::ServiceLocator::provide<core::iface::ICamera>(
		std::make_unique<infrastructure::Camera>()
	);

	// Renderer登録
	core::base::ServiceLocator::provide<core::iface::IRenderer>(
		std::make_unique<infrastructure::Renderer>()
	);

	// Animator登録
	core::base::ServiceLocator::provide<core::iface::IAnimator>(
		std::make_unique<infrastructure::Animator>()
	);

	// WindowFactory登録
	core::base::ServiceLocator::provide<core::iface::IWindowFactory>(
		std::make_unique<platform::window::WindowFactory>(*screenPtr)
	);

	// SceneManager登録（内部でSceneFactoryを所有）
	core::base::ServiceLocator::provide(
		std::make_unique<game::scene::SceneManager>()
	);

	// パフォーマンスデータプロバイダを登録
	core::base::ServiceLocator::provide<core::iface::IPerformanceDataProvider>(
		std::make_unique<platform::system::WindowsPerformanceProvider>()
	);

	// EffectFactory登録
	auto effectFactory{ std::make_unique<infrastructure::EffectFactory>() };
	effectFactory->initialize();
	core::base::ServiceLocator::provide<core::iface::IEffectFactory>(
		std::move(effectFactory)
	);
}