#pragma once
#include "IScene.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IWindow.h"
#include "core/interface/IResultWindowManager.h"
#include "core/interface/IInputProvider.h"
#include "game/ui/UiInputMapper.h"
#include <memory>

namespace game
{
	class GameManager; // 前方宣言
} // namespace game

namespace game::scene
{
    /**
     * @brief リザルトシーンのクラス
     */
    class Result : public IScene
    {
    public:
	  /**
	   * @brief Resultのコンストラクタ
	   * @param uiRenderer UI描画インターフェース
	   * @param screen 画面情報インターフェース
	   * @param resultWindow リザルトウィンドウ
	   * @param gameManager リザルトデータの取得元
	   */
	  Result(core::iface::IUIRenderer& uiRenderer,
		  core::iface::IScreen& screen,
		  std::unique_ptr<core::iface::IWindow> resultWindow,
		  GameManager& gameManager,
		  core::iface::IInputProvider& inputProvider);

	  /**
	   * @brief Resultのデストラクタ
	   */
	  ~Result() noexcept;

	  /**
	   * @brief シーンの更新処理
	   * @param deltaTime フレーム間の時間差
	   */
	  void update(float deltaTime) override;

	  /**
	   * @brief パッドの操作を読み取り、Windowへ渡す
	   *
	   * updateではなくフレーム単位で呼ぶのは、updateが固定ステップで
	   * 1フレームに0回のこともあり、押した瞬間を取りこぼすため
	   * @param deltaTime フレーム間の時間差（秒）
	   */
	  void updateInput(float deltaTime) override;

	  /**
	   * @brief シーンの描画処理
	   */
	  void draw() override;

    private:
        core::iface::IUIRenderer&              m_uiRenderer;
        core::iface::IScreen&                  m_screen;
        std::unique_ptr<core::iface::IWindow> m_resultWindow{};

		core::iface::IInputProvider& m_inputProvider;

		// パッドの操作をUI共通の意図へ翻訳する（長押しの繰り返しもここが持つ）
		ui::UiInputMapper m_inputMapper;

		// 1度でもパッドを触ったか。触るまでは枠を出さない
		bool m_hasPadFocus{ false };
	};
} // namespace game::scene