#pragma once

namespace game
{
	class GameManager;  // 前方宣言
	class PauseManager; // 前方宣言
} // namespace game

/**
 * @brief ServiceLocatorへのサービス登録を一元管理するクラス
 * ゲーム起動時に使用するサービスをここに登録する
 */
class ServiceLocatorInitializer
{
public:
  /**
   * @brief ServiceLocatorに全サービスを登録する
   * @param screenWidth 画面幅
   * @param screenHeight 画面高さ
   * @param gameManager シーン間共有データ（SceneManager経由で各シーンへ注入する）
   * @param pauseManager ポーズ状態（SceneManager経由で各シーンへ注入する）
   */
  static void init(int screenWidth, int screenHeight,
	  game::GameManager& gameManager, game::PauseManager& pauseManager);
};