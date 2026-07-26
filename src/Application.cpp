#include "Application.h"
#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IResourcePreloader.h"
#include "core/input/KeyCode.h"
#include "game/scene/SceneManager.h"
#include <DxLib.h>
#include <chrono>

namespace
{
	constexpr float TARGET_FPS{ 60.0f };
	constexpr float DELTA_TIME{ 1.0f / TARGET_FPS };

	// 1フレームで消化する更新回数の上限。
	// ブレークポイントで止めた後やロード直後は経過時間が数秒に達することがあり、
	// 制限しないとその分だけupdateを連打して復帰できなくなる（death spiral）。
	// 上限に当たった分の時間は切り捨て、ゲーム内時間が遅れることを許容する
	constexpr int MAX_UPDATES_PER_FRAME{ 5 };

	// 1フレームで先読みに使ってよい時間（ミリ秒）。シーンの性質で使い分ける。
	// モデル1件が予算を超えることもあるため、これは上限ではなく「これを超えたら次のフレームに回す」目安
	constexpr int LOOSE_PRELOAD_BUDGET_MS{ 8 };
	constexpr int HEAVY_PRELOAD_BUDGET_MS{ 12 };
	constexpr int TIGHT_PRELOAD_BUDGET_MS{ 3 };
} // namespace

Application::Application(int screenWidth, int screenHeight)
{
	// サービスを登録する（GameManager/PauseManagerはApplicationが所有し、参照を注入する）
	ServiceLocatorInitializer::init(screenWidth, screenHeight, m_gameManager, m_pauseManager);

	m_sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
	m_inputProvider = core::base::ServiceLocator::get<core::iface::IInputProvider>();
	m_preloader = core::base::ServiceLocator::get<core::iface::IResourcePreloader>();

	// 起動直後から全リソースの先読みを始める。BIOS〜Selectの間にほぼ読み終わるため、
	// InGame生成時の loadXxxById() はキャッシュヒットになりロード待ちが消える
	m_preloader->enqueueAll();

	// ポーズメニューを生成する（UIサービスの初期化後に行う）
	m_pauseMenuController = std::make_unique<game::ui::pause::PauseMenuController>(
	    *m_inputProvider,
	    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
	    *core::base::ServiceLocator::get<core::iface::IScreen>());

	// 初期シーンを設定する
	// DEBUG: リリース時はBIOSからスタートすること
	m_sceneManager->changeScene(game::scene::SceneType::InGame);
}

void Application::run()
{
	// 実経過時間を貯めてDELTA_TIMEぶん溜まるごとにupdateを回す（詳細はヘッダのコメント参照）
	auto lastFrameTime{ std::chrono::steady_clock::now() };
	float accumulator{ 0.0f };

	while (m_isRunning && !m_gameManager.isQuitRequested() && ProcessMessage() == 0)
	{
		const auto now{ std::chrono::steady_clock::now() };
		const float elapsedTime{ std::chrono::duration<float>(now - lastFrameTime).count() };
		lastFrameTime = now;

		ClearDrawScreen(); // 画面クリア

		// このフレームで使うキー入力状態を確定させる（Application/Scene/Systemの
		// どこで何度チェックしても同じ値になるようにする。詳細はIInputProvider参照）
		m_inputProvider->captureFrameInput();

		// シーンをまたぐポーズメニュー（Esc）の開閉・操作を処理する
		updatePauseMenu();

		if (m_pauseManager.isPausedBy(game::PauseReason::Menu))
		{
			// メニュー中はシーンの時間を完全に止め、止まった画面の上へメニューを重ねる。
			// 貯めた時間も捨てる（捨てないと再開した瞬間にメニューを開いていた時間ぶん早送りされる）
			accumulator = 0.0f;
			m_sceneManager->draw();
			m_pauseMenuController->draw();
		}
		else
		{
			accumulator += elapsedTime;

			int updateCount{ 0 };
			while (accumulator >= DELTA_TIME && updateCount < MAX_UPDATES_PER_FRAME)
			{
				auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
				if (audio)
					audio->update(DELTA_TIME);

				m_sceneManager->update(DELTA_TIME);
				accumulator -= DELTA_TIME;
				++updateCount;
			}

			// 上限まで回しても処理しきれていない＝処理落ちが続いている状態。
			// 残りを持ち越すと次フレーム以降も上限に張り付いて悪化するため、ここで捨てる
			if (updateCount >= MAX_UPDATES_PER_FRAME)
				accumulator = 0.0f;

			m_sceneManager->draw();
		}

		// リソースの先読みは描画の後・ScreenFlipの前に行う。
		// 1件で数百ms掛かることがあり、その時間を次フレームのelapsedTimeに混ぜると
		// accumulatorがMAX_UPDATES_PER_FRAMEに張り付いて処理落ちが連鎖するため、
		// 実際に読み込んだ場合はlastFrameTimeを取り直して先読み時間を計測から外す
		if (m_preloader->step(preloadBudgetMs(m_sceneManager->getCurrentSceneType())) > 0)
			lastFrameTime = std::chrono::steady_clock::now();

		// 入力の「前回状態」はフレームに1回だけ更新する。
		// updateの実行回数（0〜MAX_UPDATES_PER_FRAME回）に関わらず、押した瞬間の判定が
		// 1フレームにつき1回だけ成立するようにするため、updateの外側に置く
		m_inputProvider->updatePreviousState();

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
			m_pauseMenuController->open(allowBackToTitle(sceneType));
		}
	}

	if (!m_pauseManager.isPausedBy(game::PauseReason::Menu))
		return;

	// メニューの選択・決定を処理する
	switch (m_pauseMenuController->update())
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

int Application::preloadBudgetMs(game::scene::SceneType sceneType) const noexcept
{
	switch (sceneType)
	{
	// 演出を眺めるだけのシーン。カクついても気付かれにくいので多めに割く
	case game::scene::SceneType::Bios:
	case game::scene::SceneType::Title:
	case game::scene::SceneType::Lockscreen:
		return LOOSE_PRELOAD_BUDGET_MS;

	// ローディング中は残りを一気に片付けたい
	case game::scene::SceneType::Loading:
		return HEAVY_PRELOAD_BUDGET_MS;

	// WebViewの操作に追従する必要があるため絞る
	case game::scene::SceneType::Select:
		return TIGHT_PRELOAD_BUDGET_MS;

	// ゲーム中の先読みは論外（1フレームでも詰まらせない）
	default:
		return 0;
	}
}
