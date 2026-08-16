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
		/** @brief 行が取りうる値の範囲と増減の刻み */
		struct ValueRange
		{
			int m_min;
			int m_max;
			int m_step;
		};

		/**
		 * @brief マウスの移動・クリック・ドラッグを処理する
		 * @return 閉じる操作だったら true
		 */
		[[nodiscard]] bool handleMouse();

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
		 * @brief 行を決定したときの動作を行う（トグルの切り替え・リセットの実行）
		 * @param row ページ内の行番号
		 */
		void activateRow(int row);

		/**
		 * @brief スライダー上の割合を値へ直して反映する
		 * @param row ページ内の行番号
		 * @param ratio スライダー上の割合（0.0〜1.0）
		 */
		void applySliderRatio(int row, float ratio);

		/**
		 * @brief 選択中のページを既定値へ戻す
		 */
		void resetCurrentPage();

		/**
		 * @brief 指定した行の値の範囲と刻みを返す
		 * @param row ページ内の行番号
		 * @return 値の範囲と刻み
		 */
		[[nodiscard]] ValueRange getValueRange(int row) const noexcept;

		/**
		 * @brief 指定した行がいま示している値を返す
		 * @param row ページ内の行番号
		 * @return 現在の値（値を持たない行なら0）
		 */
		[[nodiscard]] int getRowValue(int row) const noexcept;

		/**
		 * @brief 指定した行へ値を書き戻す（変化があれば効果音も鳴らす）
		 * @param row ページ内の行番号
		 * @param value 新しい値
		 */
		void setRowValue(int row, int value);

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

		core::iface::IInputProvider& m_inputProvider;
		game::SettingsManager& m_settingsManager;

		UiInputMapper m_inputMapper;
		SettingsPanelView m_view;

		SettingsPage m_page{ SettingsPage::Sound };

		/** @brief 選択位置（0〜PAGE_COUNT-1 は左ナビ、それ以降がページ内の行） */
		int m_focusIndex{ 0 };

		/** @brief マウス左ボタンのエッジ検出用 */
		bool m_prevMouseLeft{ false };

		/** @brief スライダーを掴んでいる行（-1 なら掴んでいない） */
		int m_draggingRow{ -1 };
	};
} // namespace game::ui::settings
