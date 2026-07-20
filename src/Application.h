#pragma once
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/ui/pause/PauseMenu.h"
#include "game/scene/SceneType.h"
#include <memory>

namespace game::scene
{
	class SceneManager; // 前方宣言
}

namespace core::iface
{
	class IInputProvider; // 前方宣言
}

/**
 * @brief アプリケーション全体を統括する最上位クラス（コンポジションルート）
 *
 * GameManager / PauseManager の唯一のインスタンスを所有し、
 * サービスの初期化とメインループ（更新・描画・ポーズ制御）を担当する。
 * シーンをまたぐポーズメニュー（Esc）はシーンの外側であるここで処理する。
 */
class Application
{
  public:
	/**
	 * @brief Applicationのコンストラクタ（サービス初期化とシーン起動を行う）
	 * @param screenWidth 画面の幅
	 * @param screenHeight 画面の高さ
	 */
	Application(int screenWidth, int screenHeight);

	/**
	 * @brief メインループを実行する（ウィンドウが閉じられるか終了操作まで戻らない）
	 */
	void run();

  private:
	/**
	 * @brief Escキーによるポーズメニューの開閉と、メニュー操作の結果を処理する
	 */
	void updatePauseMenu();

	/**
	 * @brief 指定シーンでポーズメニューを開けるかどうかを返す
	 * @param sceneType 判定するシーンの種類
	 * @return 開ける場合true（Bios/Loading/Resultでは開けない）
	 */
	[[nodiscard]] bool canOpenPauseMenu(game::scene::SceneType sceneType) const noexcept;

	/**
	 * @brief 指定シーンで「タイトルへ戻る」を表示するかどうかを返す
	 * @param sceneType 判定するシーンの種類
	 * @return 表示する場合true（Select/InGameのみ）
	 */
	[[nodiscard]] bool allowBackToTitle(game::scene::SceneType sceneType) const noexcept;

	game::GameManager m_gameManager{};
	game::PauseManager m_pauseManager{};

	// サービス初期化後に生成するためポインタで持つ（所有はApplication）
	std::unique_ptr<game::ui::pause::PauseMenu> m_pauseMenu;

	// ServiceLocatorが所有するサービスへの参照（初期化後に取得する）
	game::scene::SceneManager* m_sceneManager{ nullptr };
	core::iface::IInputProvider* m_inputProvider{ nullptr };

	bool m_isRunning{ true };
};
