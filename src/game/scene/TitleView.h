#pragma once
#include "core/interface/IInputProvider.h"
#include "core/interface/IPerformanceDataProvider.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include <array>
#include <functional>
#include <string>

namespace game::scene
{
	/**
	 * @brief タイトル画面のグラフに出す計測チャンネル
	 */
	enum class TitleChannel
	{
		Cpu,
		Memory,
		Disk,
		Count,
	};

	/**
	 * @brief タイトルシーンの描画クラス
	 *
	 * Windows 11 のタスクマネージャー（パフォーマンスタブ）を模したウィンドウとして描く。
	 * 実物の骨格を借りたうえで、見出しの位置にはゲーム名を置く。
	 *
	 * ボタンは共通の ui::Button ではなく自前で描き、当たり判定もここが持つ。
	 * 設定画面・ポーズメニューと同じ作りで、Fluent の見た目を1か所で管理するため
	 */
	class TitleView
	{
	  public:
		/**
		 * @brief TitleViewのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 * @param resourceManager リソース管理インターフェース（アプリアイコンの読み込みに使う）
		 * @param onGoToSelect 「選択画面へ」押下時コールバック
		 * @param onOpenSettings 「設定」押下時コールバック
		 * @param onExit 「タスクを終了する」押下時コールバック
		 */
		TitleView(core::iface::IInputProvider& inputProvider,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IResourceManager& resourceManager,
		    std::function<void()> onGoToSelect,
		    std::function<void()> onOpenSettings,
		    std::function<void()> onExit);

		/**
		 * @brief 入力とパフォーマンス履歴を更新する
		 * @param snap 最新のパフォーマンススナップショット
		 * @param deltaTime 前フレームからの経過時間（秒）
		 */
		void update(const core::iface::PerformanceSnapshot& snap, float deltaTime);

		/**
		 * @brief タイトル画面を描画する
		 */
		void drawTitle() const;

		/**
		 * @brief 操作を受け付けるかを設定する
		 *
		 * フェード中は押せないようにする。描画自体は常に行う
		 * @param visible 受け付けるなら true
		 */
		void setButtonsVisible(bool visible);

	  private:
		/** @brief グラフに残す履歴の数（実物と同じく60秒ぶんの見た目にする） */
		static constexpr int HISTORY_SIZE{ 120 };

		/** @brief チャンネルの数 */
		static constexpr int CHANNEL_COUNT{ static_cast<int>(TitleChannel::Count) };

		/** @brief 押せる場所の種類 */
		enum class Hit
		{
			None,
			ThumbCpu,
			ThumbMemory,
			ThumbDisk,
			Settings,
			Exit,
			Start,
		};

		/** @brief 1チャンネルぶんの計測状態 */
		struct ChannelState
		{
			/// @brief 指数移動平均（EMA）で均した値。生値のままだと折れ線が暴れる
			float m_smoothed{};
			std::array<float, HISTORY_SIZE> m_history{};
		};

		// ---- 描画 ----
		void drawWindow() const;
		void drawTitleBar() const;
		void drawNav() const;
		void drawAppHeader() const;
		void drawThumbnails() const;
		void drawDetail() const;
		void drawGraph(int x, int y, int width, int height, int channelIndex, bool withGrid) const;
		void drawStats(int y) const;
		void drawStartButton() const;

		/**
		 * @brief 「選択画面へ」から広がる輪を描く
		 *
		 * ここを押さないとゲームが始まらないのに、白い画面では静かなボタンが埋もれる。
		 * 一定の間隔で輪を広げて、次に押す場所だと分かるようにする
		 * @param x ボタンの左上X座標
		 * @param y ボタンの左上Y座標
		 * @param width ボタンの幅
		 * @param height ボタンの高さ
		 */
		void drawStartPulse(int x, int y, int width, int height) const;

		/**
		 * @brief 歯車（設定）のアイコンを描く
		 *
		 * 実物のタスクマネージャーは左ナビの項目に必ずアイコンが付く。
		 * 画像を持たずに済むよう、輪と歯を線で組み立てる
		 * @param centerX 中心X座標
		 * @param centerY 中心Y座標
		 * @param size アイコンの一辺（px）
		 * @param color 線の色
		 */
		void drawGearIcon(int centerX, int centerY, int size, unsigned int color) const;

		/**
		 * @brief Fluent のボタンを描く
		 * @param x 左上X座標
		 * @param y 左上Y座標
		 * @param width 幅
		 * @param height 高さ
		 * @param label ラベル（UTF-8）
		 * @param isAccent アクセント色で塗るか
		 * @param isHovered カーソルが乗っているか
		 */
		void drawButton(int x, int y, int width, int height, const char* label,
		    bool isAccent, bool isHovered) const;

		// ---- レイアウト（描画と当たり判定で共有する） ----
		void updateLayout();
		void getThumbRect(int index, int& outX, int& outY, int& outWidth, int& outHeight) const;
		void getNavSettingsRect(int& outX, int& outY, int& outWidth, int& outHeight) const;
		void getExitButtonRect(int& outX, int& outY, int& outWidth, int& outHeight) const;
		void getStartButtonRect(int& outX, int& outY, int& outWidth, int& outHeight) const;

		/**
		 * @brief 指定座標にある押せる場所を返す
		 * @param x 判定するX座標
		 * @param y 判定するY座標
		 * @return 押せる場所（どこでもなければ Hit::None）
		 */
		[[nodiscard]] Hit getHitAt(int x, int y) const;

		/**
		 * @brief 基準サイズ（ウィンドウ高さ740px）での値を、いまの画面サイズへ換算する
		 * @param basePixels 基準サイズでのピクセル数
		 * @return 換算後のピクセル数（最低1）
		 */
		[[nodiscard]] int scaled(float basePixels) const noexcept;

		/**
		 * @brief UTF-8の文字列を描画用（Shift-JIS）へ変換する
		 * @param utf8 変換する文字列
		 * @return 変換後の文字列
		 */
		[[nodiscard]] std::string toDrawable(const char* utf8) const;

		/**
		 * @brief UI操作の効果音を鳴らす
		 *
		 * 3つのボタンは押した先（Title::goToSelect など）が鳴らすので、
		 * ここで鳴らすのはグラフの切り替えだけ
		 */
		void playUiClick() const;

		/**
		 * @brief 履歴を1つ進める
		 * @param buffer 対象の履歴
		 * @param value 新しく積む値
		 */
		static void pushHistory(std::array<float, HISTORY_SIZE>& buffer, float value);

		core::iface::IInputProvider& m_inputProvider;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		std::function<void()> m_onGoToSelect;
		std::function<void()> m_onOpenSettings;
		std::function<void()> m_onExit;

		/** @brief アプリアイコンの画像ハンドル（-1 なら描かない） */
		int m_iconHandle{ -1 };

		std::array<ChannelState, CHANNEL_COUNT> m_channels{};

		/** @brief 右の大きなグラフに出しているチャンネル */
		int m_selectedChannel{ static_cast<int>(TitleChannel::Cpu) };

		/** @brief 「選択画面へ」の呼び込みの経過時間（秒）。一定の周期で0へ戻す */
		float m_pulseTimer{};

		Hit m_hovered{ Hit::None };
		bool m_isInteractive{ false };
		bool m_prevMouseLeft{ false };

		// updateLayout() が求めるレイアウト（すべてピクセル）
		int m_windowX{};
		int m_windowY{};
		int m_windowWidth{};
		int m_windowHeight{};
		int m_titleBarHeight{};
		int m_navWidth{};
		int m_contentX{};
		int m_contentY{};
		int m_contentWidth{};
		int m_contentHeight{};
		int m_panesTop{};
		int m_detailX{};
		int m_detailWidth{};
		float m_scale{ 1.0f };
	};
} // namespace game::scene
