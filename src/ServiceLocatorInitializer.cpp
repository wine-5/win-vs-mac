#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IStringConverter.h"
#include "core/interface/IWindowFactory.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IAnimator.h"
#include "platform/utility/StringConverter.h"
#include "platform/window/WindowFactory.h"
#include "infrastructure/graphics/Screen.h"
#include "platform/utility/LogUtil.h"
#include "infrastructure/ResourceManager.h"
#include "infrastructure/graphics/UIRenderer.h"
#include "infrastructure/InputManager.h"
#include "infrastructure/graphics/Camera.h"
#include "infrastructure/graphics/Renderer.h"
#include "infrastructure/graphics/Animator.h"
#include "game/scene/SceneManager.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "core/interface/IPerformanceDataProvider.h"
#include "platform/system/WindowsPerformanceProvider.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IEffectFactory.h"
#include "infrastructure/AudioManager.h"
#include "infrastructure/EffectFactory.h"

void ServiceLocatorInitializer::init(int screenWidth, int screenHeight,
    game::GameManager& gameManager, game::PauseManager& pauseManager)
{
	// 文字列変換プロバイダを登録
	core::base::ServiceLocator::provide<core::iface::IStringConverter>(
		std::make_unique<platform::utility::StringConverter>()
	);

	// ResourceManager を生成して登録（Facade パターン：内部でリポジトリが管理）
	// 失敗時は握りつぶさず伝播させる。リソースが欠けたまま起動すると
	// 「モデルが出ない・音が鳴らない」状態で原因究明が遅れるため（Fail Fast）
	core::base::ServiceLocator::provide<core::iface::IResourceManager>(
	    std::make_unique<infrastructure::ResourceManager>());

	// デバッグ用ロガーを登録
	core::base::ServiceLocator::provide<core::iface::ILogger>(
	    std::make_unique<platform::utility::LogUtil>());

	// Screen登録（SetGraphMode()で設定した画面サイズを渡す）
	auto screen = std::make_unique<infrastructure::graphics::Screen>(screenWidth, screenHeight);
	auto* screenPtr = screen.get();
	core::base::ServiceLocator::provide<core::iface::IScreen>(
		std::move(screen)
	);

	// UIRenderer登録
	core::base::ServiceLocator::provide<core::iface::IUIRenderer>(
	    std::make_unique<infrastructure::graphics::UIRenderer>());

	// InputManager登録
	core::base::ServiceLocator::provide<core::iface::IInputProvider>(
		std::make_unique<infrastructure::InputManager>()
	);

	// Camera登録
	core::base::ServiceLocator::provide<core::iface::ICamera>(
	    std::make_unique<infrastructure::graphics::Camera>());

	// Renderer登録
	core::base::ServiceLocator::provide<core::iface::IRenderer>(
	    std::make_unique<infrastructure::graphics::Renderer>());

	// Animator登録
	core::base::ServiceLocator::provide<core::iface::IAnimator>(
	    std::make_unique<infrastructure::graphics::Animator>());

	// WindowFactory登録
	core::base::ServiceLocator::provide<core::iface::IWindowFactory>(
		std::make_unique<platform::window::WindowFactory>(*screenPtr)
	);

	// SceneManager登録（内部でSceneFactoryを所有。GameManager/PauseManagerを各シーンへ注入する）
	core::base::ServiceLocator::provide(
	    std::make_unique<game::scene::SceneManager>(gameManager, pauseManager));

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

	// AudioManager登録
	auto audioManager{ std::make_unique<infrastructure::AudioManager>() };
	audioManager->initialize();
	core::base::ServiceLocator::provide<core::iface::IAudioManager>(
		std::move(audioManager)
	);
}