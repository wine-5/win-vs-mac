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
#include "core/interface/IWindowFactory.h"
#include "core/interface/ISelectWindowManager.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"

namespace game::scene
{
	SceneFactory::SceneFactory(GameManager& gameManager, PauseManager& pauseManager)
	    : m_gameManager{ gameManager }
	    , m_pauseManager{ pauseManager }
	{
	}

	SceneFactory::~SceneFactory() = default;

	IScene* SceneFactory::createScene(SceneType sceneType)
	{
		// TODO: 現在は全て軽量なシーンであるためシーンが作られるたびに生成をしているが、
		// 本来であればreset()関数をInGameなどの状態をゲームループするたびにリセットする必要がある
		// オブジェクトに持たせる必要があり、逆にリセットが不要なシーンに関しては
		// if文で重複して何度も生成されないようにするべき
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
				*screen);
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
			    [this, selectPtr = m_selectScene.get()](core::constant::JobType jt)
			    {
				    m_gameManager.getJobSelectionData().setSelectedJobType(jt);
				    selectPtr->notifyJobSelected(jt);
			    },
			    [this](int slot, const std::string& path)
			    {
				    m_gameManager.getFileEquipmentData().setFilePath(slot, path);
			    },
			    *resourceManager);

			m_selectScene->setWindowManager(std::move(windowManager));
			return m_selectScene.get();
		}

		case SceneType::Loading:
		{
			auto* uiRenderer = core::base::ServiceLocator::get<core::iface::IUIRenderer>();
			auto* windowFactory = core::base::ServiceLocator::get<core::iface::IWindowFactory>();

			auto loadingWindow = windowFactory->createLoadingWindow(
				[this]() {
					if (m_loadingScene)
						m_loadingScene->notifyLoadingComplete();
				});

			m_loadingScene = std::make_unique<Loading>(
				*uiRenderer,
				*screen,
				std::move(loadingWindow));
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
			    m_pauseManager);
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
