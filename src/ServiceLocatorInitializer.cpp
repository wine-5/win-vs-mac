#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IResourcePreloader.h"
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
#include "infrastructure/resource/ResourceManager.h"
#include "infrastructure/resource/ResourcePreloader.h"
#include "infrastructure/graphics/UIRenderer.h"
#include "infrastructure/InputManager.h"
#include "infrastructure/graphics/Camera.h"
#include "infrastructure/graphics/Renderer.h"
#include "core/interface/ILighting.h"
#include "core/interface/IShadowMap.h"
#include "infrastructure/graphics/Lighting.h"
#include "infrastructure/graphics/ShadowMap.h"
#include "infrastructure/graphics/Animator.h"
#include "game/scene/SceneManager.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/SettingsManager.h"
#include "core/interface/IPerformanceDataProvider.h"
#include "platform/system/WindowsPerformanceProvider.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IEffectFactory.h"
#include "infrastructure/AudioManager.h"
#include "infrastructure/effect/EffectFactory.h"
#include "core/utility/Probe.h" // 一時: メモリ調査用（原因特定後に削除）

void ServiceLocatorInitializer::init(int screenWidth, int screenHeight,
    game::GameManager& gameManager, game::PauseManager& pauseManager,
    game::SettingsManager& settingsManager, std::function<void()> onOpenSettings)
{
	// 文字列変換プロバイダを登録
	core::base::ServiceLocator::provide<core::iface::IStringConverter>(
		std::make_unique<platform::utility::StringConverter>()
	);

	core::probe::mark("  service: StringConverter");

	// ResourceManager を生成して登録（Facade パターン：内部でリポジトリが管理）
	// 失敗時は握りつぶさず伝播させる。リソースが欠けたまま起動すると
	// 「モデルが出ない・音が鳴らない」状態で原因究明が遅れるため（Fail Fast）
	auto resourceManager{ std::make_unique<infrastructure::resource::ResourceManager>() };
	auto* resourceManagerPtr{ resourceManager.get() };
	core::base::ServiceLocator::provide<core::iface::IResourceManager>(std::move(resourceManager));

	// リソース先読みを登録（キューを積むのは Application 側）
	core::base::ServiceLocator::provide<core::iface::IResourcePreloader>(
	    std::make_unique<infrastructure::resource::ResourcePreloader>(*resourceManagerPtr));

	core::probe::mark("  service: ResourceManager");

	// デバッグ用ロガーを登録
	core::base::ServiceLocator::provide<core::iface::ILogger>(
	    std::make_unique<platform::utility::LogUtil>());

	core::probe::mark("  service: Logger");

	// Screen登録（SetGraphMode()で設定した画面サイズを渡す）
	auto screen = std::make_unique<infrastructure::graphics::Screen>(screenWidth, screenHeight);
	auto* screenPtr = screen.get();
	core::base::ServiceLocator::provide<core::iface::IScreen>(
		std::move(screen)
	);

	core::probe::mark("  service: Screen");

	// UIRenderer登録
	core::base::ServiceLocator::provide<core::iface::IUIRenderer>(
	    std::make_unique<infrastructure::graphics::UIRenderer>());

	core::probe::mark("  service: UIRenderer");

	// InputManager登録
	core::base::ServiceLocator::provide<core::iface::IInputProvider>(
		std::make_unique<infrastructure::InputManager>()
	);

	core::probe::mark("  service: InputManager");

	// Camera登録
	core::base::ServiceLocator::provide<core::iface::ICamera>(
	    std::make_unique<infrastructure::graphics::Camera>());

	core::probe::mark("  service: Camera");

	// Renderer登録
	core::base::ServiceLocator::provide<core::iface::IRenderer>(
	    std::make_unique<infrastructure::graphics::Renderer>());

	core::probe::mark("  service: Renderer");

	// Animator登録
	core::base::ServiceLocator::provide<core::iface::IAnimator>(
	    std::make_unique<infrastructure::graphics::Animator>());

	core::probe::mark("  service: Animator");

	// Lighting登録
	core::base::ServiceLocator::provide<core::iface::ILighting>(
	    std::make_unique<infrastructure::graphics::Lighting>());

	core::probe::mark("  service: Lighting");

	// ShadowMap登録。ハンドルの生成はシーン側（使う所）で行うため、ここでは器だけ用意する
	core::base::ServiceLocator::provide<core::iface::IShadowMap>(
	    std::make_unique<infrastructure::graphics::ShadowMap>());

	core::probe::mark("  service: ShadowMap");

	// WindowFactory登録
	core::base::ServiceLocator::provide<core::iface::IWindowFactory>(
		std::make_unique<platform::window::WindowFactory>(*screenPtr)
	);

	core::probe::mark("  service: WindowFactory");

	// SceneManager登録（内部でSceneFactoryを所有。横断データを各シーンへ注入する）
	core::base::ServiceLocator::provide(
	    std::make_unique<game::scene::SceneManager>(gameManager, pauseManager, settingsManager,
	        std::move(onOpenSettings)));

	core::probe::mark("  service: SceneManager");

	// パフォーマンスデータプロバイダを登録
	core::base::ServiceLocator::provide<core::iface::IPerformanceDataProvider>(
		std::make_unique<platform::system::WindowsPerformanceProvider>()
	);

	core::probe::mark("  service: PerformanceProvider");

	// EffectFactory登録
	auto effectFactory{ std::make_unique<infrastructure::effect::EffectFactory>() };
	effectFactory->initialize();
	core::base::ServiceLocator::provide<core::iface::IEffectFactory>(
		std::move(effectFactory)
	);

	core::probe::mark("  service: EffectFactory");

	// AudioManager登録
	auto audioManager{ std::make_unique<infrastructure::AudioManager>() };
	audioManager->initialize();
	core::base::ServiceLocator::provide<core::iface::IAudioManager>(
		std::move(audioManager)
	);

	core::probe::mark("  service: AudioManager");
}
