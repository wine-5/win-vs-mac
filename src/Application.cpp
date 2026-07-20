#include "Application.h"
#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IAudioManager.h"
#include "core/input/KeyCode.h"
#include "game/scene/SceneManager.h"
#include <DxLib.h>

namespace
{
	constexpr float TARGET_FPS{ 60.0f };
	constexpr float DELTA_TIME{ 1.0f / TARGET_FPS };
} // namespace

Application::Application(int screenWidth, int screenHeight)
{
	// サービスを登録する（GameManager/PauseManagerはApplicationが所有し、参照を注入する）
	ServiceLocatorInitializer::init(screenWidth, screenHeight, m_gameManager, m_pauseManager);

	m_sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
	m_inputProvider = core::base::ServiceLocator::get<core::iface::IInputProvider>();

	// ポーズメニューを生成する（UIサービスの初期化後に行う）
	m_pauseMenu = std::make_unique<game::ui::pause::PauseMenu>(
	    *m_inputProvider,
	    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
	    *core::base::ServiceLocator::get<core::iface::IScreen>());

	// 初期シーンを設定する
	// DEBUG: リリース時はBIOSからスタートすること
	m_sceneManager->changeScene(game::scene::SceneType::InGame);
}

void Application::run()
{
	while (m_isRunning && ProcessMessage() == 0)
	{
		ClearDrawScreen(); // 画面クリア

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->update();

		// シーンをまたぐポーズメニュー（Esc）の開閉・操作を処理する
		updatePauseMenu();

		if (m_pauseManager.isPausedBy(game::PauseReason::Menu))
		{
			// メニュー中はシーンの時間を完全に止め、止まった画面の上へメニューを重ねる
			m_sceneManager->draw();
			m_pauseMenu->draw();

			// シーンのupdateを飛ばすため、入力のフレーム更新はここで行う
			m_inputProvider->updatePreviousState();
		}
		else
		{
			m_sceneManager->update(DELTA_TIME);
			m_sceneManager->draw();
		}

		ScreenFlip(); // 画面を反映
	}
}

void Application::updatePauseMenu()
{
	const auto sceneType{ m_sceneManager->getCurrentSceneType() };

	// Escで開閉する（別の理由（シーンビュー等）でポーズ中は何もしない）
	if (m_inputProvider->isKeyPressed(core::input::KeyCode::Escape))
	{
		if (m_pauseManager.isPausedBy(game::PauseReason::Menu))
			m_pauseManager.resume();
		else if (!m_pauseManager.isPaused() && canOpenPauseMenu(sceneType))
		{
			m_pauseManager.pause(game::PauseReason::Menu);
			m_pauseMenu->open(allowBackToTitle(sceneType));
		}
	}

	if (!m_pauseManager.isPausedBy(game::PauseReason::Menu))
		return;

	// メニューの選択・決定を処理する
	switch (m_pauseMenu->update())
	{
	case game::ui::pause::PauseMenuAction::Resume:
		m_pauseManager.resume();
		break;

	case game::ui::pause::PauseMenuAction::BackToTitle:
		m_pauseManager.resume();
		m_sceneManager->changeScene(game::scene::SceneType::Title);
		break;

	case game::ui::pause::PauseMenuAction::Quit:
		m_isRunning = false;
		break;

	default:
		break;
	}
}

bool Application::canOpenPauseMenu(game::scene::SceneType sceneType) const noexcept
{
	// Bios（Escをスキップに使用）・Loading（中断不可）・Result（専用UIあり）では開かない
	switch (sceneType)
	{
	case game::scene::SceneType::Title:
	case game::scene::SceneType::Lockscreen:
	case game::scene::SceneType::Select:
	case game::scene::SceneType::InGame:
		return true;
	default:
		return false;
	}
}

bool Application::allowBackToTitle(game::scene::SceneType sceneType) const noexcept
{
	// タイトルより後のシーンでのみ「タイトルへ戻る」を表示する
	return sceneType == game::scene::SceneType::Select ||
	       sceneType == game::scene::SceneType::InGame;
}
