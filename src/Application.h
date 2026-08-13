#pragma once
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/ui/pause/PauseMenuController.h"
#include "game/scene/SceneType.h"
#include "core/constant/SeType.h"
#include <memory>

namespace game::scene
{
	class SceneManager; // 前方宣言
}

namespace core::iface
{
	class IInputProvider;     // 前方宣言
	class IResourcePreloader; // 前方宣言
} // namespace core::iface

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
	 *
	 * 更新は固定タイムステップ（1/60秒）で行い、描画はフレームに1回行う。
	 * ScreenFlipはVSync待ちのため描画のフレームレートはモニタのリフレッシュレートに
	 * 引きずられる（120Hzのモニタなら毎秒120フレーム）。updateまで同じ回数呼ぶと
	 * ゲームの進行速度がモニタ依存で変わってしまうため、実経過時間を貯めておき
	 * 1/60秒ぶん溜まるごとにupdateを回す。これによりモニタが何Hzでも
	 * ゲーム内時間の進み方と物理・当たり判定の挙動が一定になる。
	 */
	void run();

  private:
	/**
	 * @brief Escキーによるポーズメニューの開閉と、メニュー操作の結果を処理する
	 */
	void updatePauseMenu();

	/**
	 * @brief UI操作の効果音を鳴らす
	 *
	 * ポーズの開閉はシーンをまたいで同じ操作なので、シーン側ではなくここが鳴らす
	 * @param seType 鳴らすSEの種別
	 */
	void playUiSe(core::constant::SeType seType) const;

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

	/**
	 * @brief 指定シーンで1フレームあたり先読みに使ってよい時間を返す
	 *
	 * 静止画面のシーンほど多く割き、操作に追従する必要があるシーンでは絞る。
	 * InGameは0（＝先読みを止める）。
	 * @param sceneType 判定するシーンの種類
	 * @return フレーム予算（ミリ秒）
	 */
	[[nodiscard]] int preloadBudgetMs(game::scene::SceneType sceneType) const noexcept;

	game::GameManager m_gameManager{};
	game::PauseManager m_pauseManager{};

	// サービス初期化後に生成するためポインタで持つ（所有はApplication）
	std::unique_ptr<game::ui::pause::PauseMenuController> m_pauseMenuController;

	// ServiceLocatorが所有するサービスへの参照（初期化後に取得する）
	game::scene::SceneManager* m_sceneManager{ nullptr };
	core::iface::IInputProvider* m_inputProvider{ nullptr };
	core::iface::IResourcePreloader* m_preloader{ nullptr };

	bool m_isRunning{ true };
};
