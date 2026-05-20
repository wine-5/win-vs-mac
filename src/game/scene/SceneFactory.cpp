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
#include "core/interface/IFileProvider.h"
#include "core/interface/IResourceManager.h"
#include "game/GameManager.h"
#include "platform/window/select/Win32SelectWindowManager.h"

namespace
{
	constexpr const char* MAIN_FONT_NAME = "x12y16pxMaruMonica";
}

namespace game::scene
{
	SceneFactory::SceneFactory()
		: m_inGameScene{}
		, m_titleScene{}
		, m_lockscreenScene{}
		, m_selectScene{}
		, m_loadingScene{}
		, m_resultScene{}		, m_biosScene{}
		, m_biosUIRenderer{ MAIN_FONT_NAME }		, m_titleUIRenderer{ MAIN_FONT_NAME }
		, m_lockscreenUIRenderer{ MAIN_FONT_NAME }
		, m_selectUIRenderer{ MAIN_FONT_NAME }
		, m_loadingUIRenderer{ MAIN_FONT_NAME }
		, m_resultUIRenderer{ MAIN_FONT_NAME }
	{
	}

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
			m_biosScene = std::make_unique<Bios>(
				m_biosInputManager,
				m_biosUIRenderer,
				*screen);
			return m_biosScene.get();

		case SceneType::Title:
			m_titleScene = std::make_unique<Title>(
				m_titleInputManager,
				m_titleUIRenderer,
				*screen);
			return m_titleScene.get();

		case SceneType::Lockscreen:
			m_lockscreenScene = std::make_unique<Lockscreen>(
				m_lockscreenInputManager,
				m_lockscreenUIRenderer,
				*screen);
			return m_lockscreenScene.get();

		case SceneType::Select:
		{
			auto* fileProvider = core::base::ServiceLocator::get<core::iface::IFileProvider>();
			auto* resourceManager = core::base::ServiceLocator::get<core::iface::IResourceManager>();

			m_selectScene = std::make_unique<Select>(
				m_selectInputManager,
				m_selectUIRenderer,
				*screen,
				*fileProvider,
				*resourceManager,
				game::GameManager::getInstance().getFileEquipmentData(),
				nullptr);

			auto windowManager = std::make_unique<platform::window::select::Win32SelectWindowManager>(
				[selectPtr = m_selectScene.get()]() { selectPtr->notifyGameStart(); },
				[selectPtr = m_selectScene.get()](core::constant::JobType jt) {
					game::GameManager::getInstance().getJobSelectionData().setSelectedJobType(jt);
					selectPtr->notifyJobSelected(jt);
				},
				[fileProvider](int slot, const std::string& path) {
					game::GameManager::getInstance().getFileEquipmentData().setFilePath(slot, path);
				},
				*resourceManager,
				*screen
			);

			m_selectScene->setWindowManager(std::move(windowManager));
			return m_selectScene.get();
		}

		case SceneType::Loading:
		{
			m_loadingScene = std::make_unique<Loading>(
				m_loadingUIRenderer,
				*screen);
			return m_loadingScene.get();
		}

		case SceneType::InGame:
		{
			auto* resourceManager = core::base::ServiceLocator::get<core::iface::IResourceManager>();

			m_inGameScene = std::make_unique<InGame>(
				m_inGameCamera,
				m_inGameRenderer,
				m_inGameAnimator,
				*resourceManager,
				m_inGameInputManager,
				game::GameManager::getInstance().getFileEquipmentData());
			return m_inGameScene.get();
		}

		case SceneType::Result:
		{
			m_resultScene = std::make_unique<Result>(
				m_resultUIRenderer,
				*screen);
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
		case SceneType::Select:  m_selectScene.reset();  break;
		case SceneType::Loading: m_loadingScene.reset(); break;
		case SceneType::InGame:  m_inGameScene.reset();  break;
		case SceneType::Result:  m_resultScene.reset();  break;
		default: break;
		}
	}
}
