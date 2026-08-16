#include "SceneFactory.h"
#include "Bios.h"
#include "Title.h"
#include "Lockscreen.h"
#include "Select.h"
#include "Loading.h"
#include "InGame.h"
#include "Result.h"
#include "SceneManager.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IResourcePreloader.h"
#include "core/interface/IWindowFactory.h"
#include "core/interface/ISelectWindowManager.h"
#include "core/data/Difficulty.h"
#include "core/utility/Log.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/SettingsManager.h"

namespace
{
	/// @brief ローディング演出の再生速度（先読みが間に合っていない場合）
	constexpr float NORMAL_LOADING_SPEED{ 1.0f };

	/// @brief ローディング演出の再生速度（先読みが完了済みの場合）
	constexpr float PRELOADED_LOADING_SPEED{ 2.0f };
} // namespace

namespace game::scene
{
	SceneFactory::SceneFactory(GameManager& gameManager, PauseManager& pauseManager, SettingsManager& settingsManager,
	    std::function<void()> onOpenSettings)
	    : m_gameManager{ gameManager }
	    , m_pauseManager{ pauseManager }
	    , m_settingsManager{ settingsManager }
	    , m_onOpenSettings{ std::move(onOpenSettings) }
	{
	}

	SceneFactory::~SceneFactory() = default;

	IScene* SceneFactory::createScene(SceneType sceneType)
	{
		// 方針: シーンは遷移のたびに作り直す（インスタンスをキャッシュしない）。
		// 作り直し方式には「状態のリセット漏れが構造的に起き得ない」という利点があり、
		// キャッシュを導入すると全シーンにreset()を実装・保守する義務が新たに発生する。
		// 生成が重くなった場合はシーンの再利用ではなく、ResourceManager側のキャッシュで対処する。
		auto* screen = core::base::ServiceLocator::get<core::iface::IScreen>();

		switch (sceneType)
		{
		case SceneType::Bios:
		{
			auto* inputProvider = core::base::ServiceLocator::get<core::iface::IInputProvider>();
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			m_biosScene = std::make_unique<Bios>(
				*inputProvider,
				*uiRenderer,
				*screen);
			return m_biosScene.get();
		}

		case SceneType::Title:
		{
			auto* inputProvider = core::base::ServiceLocator::get<core::iface::IInputProvider>();
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			m_titleScene = std::make_unique<Title>(
			    *inputProvider,
			    *uiRenderer,
			    *screen,
			    m_gameManager,
			    m_onOpenSettings);
			return m_titleScene.get();
		}

		case SceneType::Lockscreen:
		{
			auto* inputProvider = core::base::ServiceLocator::get<core::iface::IInputProvider>();
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			m_lockscreenScene = std::make_unique<Lockscreen>(
				*inputProvider,
				*uiRenderer,
				*screen);
			return m_lockscreenScene.get();
		}

		case SceneType::Select:
		{
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			auto* resourceManager = core::base::ServiceLocator::get<core::iface::IResourceManager>();

			m_selectScene = std::make_unique<Select>(
			    *uiRenderer,
			    *screen,
			    *resourceManager,
			    nullptr);

			auto* windowFactory = core::base::ServiceLocator::get<core::iface::IWindowFactory>();
			auto windowManager = windowFactory->createSelectWindowManager(
			    [selectPtr = m_selectScene.get()]()
			    { selectPtr->notifyGameStart(); },
			    [selectPtr = m_selectScene.get()]()
			    { selectPtr->notifyBackToTitle(); },
			    [this]()
			    { m_gameManager.requestQuit(); },
			    [this](int slot, const std::string& path)
			    {
				    m_gameManager.getFileEquipmentData().setFilePath(slot, path);
			    },
			    [this](const std::string& difficulty)
			    {
				    m_gameManager.setDifficulty(core::data::toDifficulty(difficulty));
				    core::log::info("難易度を選択しました: {}", difficulty.c_str());
			    },
			    *resourceManager,
			    // 初見の「何をすればいいのか分からない」を解くための案内。起動後の1回だけ出す
			    m_gameManager.consumeSelectTutorial());

			m_selectScene->setWindowManager(std::move(windowManager));
			return m_selectScene.get();
		}

		case SceneType::Loading:
		{
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			auto* windowFactory = core::base::ServiceLocator::get<core::iface::IWindowFactory>();

			auto* preloader = core::base::ServiceLocator::get<core::iface::IResourcePreloader>();

			// 起動時からの先読みが済んでいれば待つ理由が無いので演出を倍速で流す
			const float loadingSpeed{ preloader->isComplete()
				                          ? PRELOADED_LOADING_SPEED
				                          : NORMAL_LOADING_SPEED };

			auto loadingWindow = windowFactory->createLoadingWindow(
			    [this]()
			    {
					if (m_loadingScene)
						m_loadingScene->notifyLoadingComplete();
			    },
			    loadingSpeed);

			m_loadingScene = std::make_unique<Loading>(
			    *uiRenderer,
			    *screen,
			    std::move(loadingWindow),
			    *preloader);
			return m_loadingScene.get();
		}

		case SceneType::InGame:
		{
			auto* camera = core::base::ServiceLocator::get<core::iface::ICamera>();
			auto* renderer = core::base::ServiceLocator::get<core::iface::IRenderer>();
			auto* animator = core::base::ServiceLocator::get<core::iface::IAnimator>();
			auto* resourceManager = core::base::ServiceLocator::get<core::iface::IResourceManager>();
			auto* inputProvider = core::base::ServiceLocator::get<core::iface::IInputProvider>();

			m_inGameScene = std::make_unique<InGame>(
			    *camera,
			    *renderer,
			    *animator,
			    *resourceManager,
			    *inputProvider,
			    m_gameManager,
			    m_pauseManager,
			    m_settingsManager);
			return m_inGameScene.get();
		}

		case SceneType::Result:
		{
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			auto* windowFactory = core::base::ServiceLocator::get<core::iface::IWindowFactory>();

			auto resultWindow = windowFactory->createResultWindow(
				[this]() {
					auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
					if (sceneManager)
						sceneManager->changeScene(SceneType::Select);
				},
				[this]() {
					auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
					if (sceneManager)
						sceneManager->changeScene(SceneType::Title);
				});

			m_resultScene = std::make_unique<Result>(
			    *uiRenderer,
			    *screen,
			    std::move(resultWindow),
			    m_gameManager);

			return m_resultScene.get();
		}

		default:
			return nullptr;
		}
	}

	void SceneFactory::resetScene(SceneType sceneType) noexcept
	{
		// 指定シーンを破棄してウィンドウなどのリソースを解放する
		switch (sceneType)
		{
		case SceneType::Bios:       m_biosScene.reset();       break;
		case SceneType::Title:      m_titleScene.reset();      break;
		case SceneType::Lockscreen: m_lockscreenScene.reset(); break;
		case SceneType::Select:     m_selectScene.reset();     break;
		case SceneType::Loading:
			m_loadingScene.reset();
			break;
		case SceneType::InGame:     m_inGameScene.reset();     break;
		case SceneType::Result:
			m_resultScene.reset();
			break;
		default: break;
		}
	}
} // namespace game::scene
