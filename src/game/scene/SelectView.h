#pragma once
#include "game/ui/UIManager.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "game/data/FileEquipmentData.h"
#include "game/data/JobSelectionData.h"
#include <functional>
#include <memory>

namespace game::ui
{
	class Button;
}

namespace game::scene
{
	/**
	 * @brief 選択シーンの描画クラス
	 */
	class SelectView
	{
	public:
		/**
		 * @brief SelectViewのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 * @param fileEquipmentData 選択ファイルデータ（表示用）
		 * @param resourceManager リソース管理インターフェース
		 * @param jobSelectionData ジョブ選択データ
		 * @param onGameStart ゲームスタートボタン押下時コールバック
		 * @param onFileSelect ファイル選択ボタン押下時コールバック（スロット番号）
		 * @param onJobSelect ジョブ選択ボタン押下時コールバック（ジョブID）
		 */
		SelectView(core::iface::IInputProvider& inputProvider,
			core::iface::IUIRenderer& uiRenderer,
			core::iface::IScreen& screen,
			data::FileEquipmentData& fileEquipmentData,
			core::iface::IResourceManager& resourceManager,
			data::JobSelectionData& jobSelectionData,
			std::function<void()> onGameStart,
			std::function<void(int)> onFileSelect,
			std::function<void(int)> onJobSelect);

		/**
		 * @brief ボタン入力を更新する
		 */
		void update();

		/**
		 * @brief 描画処理
		 */
		void draw() const;

	private:
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen&     m_screen;
		data::FileEquipmentData&  m_fileEquipmentData;
		core::iface::IResourceManager& m_resourceManager;
		data::JobSelectionData&   m_jobSelectionData;
		ui::UIManager             m_uiManager{};

		static constexpr float TITLE_Y_RATIO            = 0.12f;
		static constexpr float START_BUTTON_Y_RATIO      = 0.60f;
		static constexpr float BUTTON_WIDTH_RATIO        = 0.15f;
		static constexpr float BUTTON_HEIGHT_RATIO       = 0.06f;
		static constexpr float FILE_BUTTON_BASE_Y_RATIO  = 0.50f;
		static constexpr float FILE_BUTTON_Y_STEP        = 0.08f;
		static constexpr float FILE_NAME_BASE_Y_RATIO    = 0.43f;
		static constexpr float FILE_NAME_Y_STEP          = 0.04f;
		static constexpr int   FILE_NAME_X               = 10;
	};
} // namespace game::scene