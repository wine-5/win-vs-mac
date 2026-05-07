#pragma once
#include "game/ui/UIManager.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "game/data/FileEquipmentData.h"
#include <memory>
#include <string>

namespace game::ui
{
	class Button;
}

namespace game::scene
{
	/**
	 * @brief 選択シーンの描画・UI管理クラス
	 */
	class SelectView
	{
	public:
		enum class Action
		{
			None,
			GoToLoading
		};

		/**
		 * @brief SelectViewのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面サイズインターフェース
		 * @param fileProvider ファイル選択インターフェース
		 * @param fileEquipmentData 選択ファイルデータの参照
		 */
		SelectView(core::iface::IInputProvider& inputProvider,
			core::iface::IUIRenderer& uiRenderer,
			core::iface::IScreen& screen,
			core::iface::IFileProvider& fileProvider,
			data::FileEquipmentData& fileEquipmentData);

		/**
		 * @brief 更新処理
		 * @param deltaTime 経過時間（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief 描画処理
		 */
		void draw();

		/**
		 * @brief シーン切替の準備ができているか
		 * @return フェードアウト完了時にtrue
		 */
		[[nodiscard]] bool isReadyToChange() const;

		/**
		 * @brief 次に行うアクションを取得する
		 * @return アクション種別
		 */
		[[nodiscard]] Action getNextAction() const;

	private:
		enum class State
		{
			FadeIn,
			Idle,
			FadeOut
		};

		void setupUI();
		void startFadeOut();

		core::iface::IInputProvider& m_inputProvider;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::iface::IFileProvider& m_fileProvider;
		data::FileEquipmentData& m_fileEquipmentData;
		ui::UIManager m_uiManager{};
		std::unique_ptr<ui::FadeTransition> m_fade{};
		State m_state{ State::FadeIn };
		Action m_nextAction{ Action::None };
	};
}