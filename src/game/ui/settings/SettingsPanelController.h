#pragma once
#include "core/constant/SeType.h"
#include "game/ui/UiInputMapper.h"
#include "game/ui/settings/SettingsPanelView.h"

namespace game
{
	class SettingsManager; // 前方宣言
}

namespace game::ui::settings
{
	/**
	 * @brief 設定画面の操作結果
	 */
	enum class SettingsPanelAction
	{
		None,  // まだ開いたまま
		Close, // 閉じる
	};

	/**
	 * @brief 設定画面の入力・選択状態を制御するコントローラ
	 *
	 * Application が所有し、タイトル・ポーズのどちらから開いても同じものを使う。
	 * 値の保管と保存は SettingsManager が持ち、ここは「どこを選んでいるか」だけを持つ
	 */
	class SettingsPanelController
	{
	  public:
		/**
		 * @brief SettingsPanelControllerのコンストラクタ
		 * @param inputProvider 入力のインターフェース
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param settingsManager 設定の保管先
		 */
		SettingsPanelController(core::iface::IInputProvider& inputProvider,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    game::SettingsManager& settingsManager);

		/**
		 * @brief 画面を開く（選択位置を初期化する）
		 */
		void open();

		/**
		 * @brief 入力を処理し、閉じるべきかを返す
		 * @param deltaTime フレーム間の時間差（秒）
		 * @return 閉じるなら SettingsPanelAction::Close
		 */
		[[nodiscard]] SettingsPanelAction update(float deltaTime);

		/**
		 * @brief 画面を描画する
		 */
		void draw();

	  private:
		/**
		 * @brief 選択位置を上下に動かす
		 * @param delta 移動量（-1 で上、+1 で下）
		 */
		void moveFocus(int delta);

		/**
		 * @brief 選択中の行の値を増減する
		 * @param direction -1 で減、+1 で増
		 */
		void adjustValue(int direction);

		/**
		 * @brief 選択中の行を決定する（トグルの切り替え・リセットの実行）
		 */
		void confirm();

		/**
		 * @brief 選択中のページを既定値へ戻す
		 */
		void resetCurrentPage();

		/**
		 * @brief いま左ナビを選んでいるかを返す
		 * @return 左ナビを選んでいれば true
		 */
		[[nodiscard]] bool isNavFocused() const noexcept
		{
			return m_focusIndex < PAGE_COUNT;
		}

		/**
		 * @brief 選択中の行番号を返す（左ナビを選んでいるときは負）
		 * @return ページ内での行番号
		 */
		[[nodiscard]] int getFocusedRow() const noexcept
		{
			return m_focusIndex - PAGE_COUNT;
		}

		/**
		 * @brief UI操作の効果音を鳴らす
		 *
		 * 効果音のスライダーを動かすたびにこれが鳴るので、
		 * 変更後の音量がその場で耳で確かめられる
		 * @param seType 鳴らすSEの種別
		 */
		void playUiSe(core::constant::SeType seType) const;

		game::SettingsManager& m_settingsManager;

		UiInputMapper m_inputMapper;
		SettingsPanelView m_view;

		SettingsPage m_page{ SettingsPage::Sound };

		/** @brief 選択位置（0〜PAGE_COUNT-1 は左ナビ、それ以降がページ内の行） */
		int m_focusIndex{ 0 };
	};
} // namespace game::ui::settings
