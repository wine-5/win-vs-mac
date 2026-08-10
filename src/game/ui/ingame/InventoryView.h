#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include "core/data/FileExtensionType.h"
#include "game/utility/PlayerStats.h"
#include "HudPanel.h"
#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace game::data
{
	class FileEquipmentData; // 前方宣言
} // namespace game::data

namespace game::ui::ingame
{
	/**
	 * @brief Eキーで開く拡張子インベントリを描画するView
	 *
	 * Windowsのエクスプローラーの窓として見せる。タイトルバー・アドレスバー・
	 * マス目のグリッド・ステータスバーを備え、拡張子を「フォルダに入っている
	 * ファイル」として並べる。
	 *
	 * ただアイコンを並べるとHUDの延長にしか見えず、持ち物を覗いている感じが出ない。
	 * 窓の体裁とマス目があることで「置き場」に見え、ファイルを扱うゲームだという
	 * 世界観にもそのまま繋がる。
	 *
	 * 拡張子は出どころと状態で3つに分ける。
	 *   持ち込み   … セレクト画面で選んだもの（プレイ中は変えられない）
	 *   道中で拾った… 効果が乗っているもの
	 *   未装備     … 拾ったが枠が埋まっていて効果が乗っていないもの
	 * 「未装備」があることが、リネームブロックを探す動機になる。
	 *
	 * 開いている間は時間が止まる（PauseReason::Inventory）。読む画面なので、
	 * 読んでいる最中に殴られるのはプレイヤーの落ち度ではなく設計の落ち度になる。
	 */
	class InventoryView
	{
	  public:
		/**
		 * @brief InventoryViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager インベントリ・能力値の読み出しに使うComponentManagerの参照
		 * @param resourceManager アイコン画像の読み込みに使うリソース管理インターフェース
		 * @param equipmentData セレクト画面で選んだファイル（持ち込み）
		 */
		InventoryView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    core::iface::IResourceManager& resourceManager,
		    const data::FileEquipmentData& equipmentData);

		/**
		 * @brief インベントリを描画する
		 * @param playerId プレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

		/**
		 * @brief 付け替え操作の選択位置を設定する
		 *
		 * 位置の管理と入れ替えの実行はシーン側が持ち、こちらは受け取った位置を
		 * 描くだけにする。操作をキーからマウスへ変えても描画側を触らずに済む。
		 * @param cursorIndex 今いる位置（ExtensionInventoryComponent::m_acquired 上の添字）。
		 *                    -1 なら選択そのものを表示しない
		 * @param heldIndex 掴んでいる位置（同上）。-1 なら何も掴んでいない
		 */
		void setSelection(int cursorIndex, int heldIndex) noexcept;

		/**
		 * @brief 付け替え操作を受け付ける状態かを設定する
		 *
		 * 選択位置とは別に持つ。マウスがマスの外にある間は選択位置が無くなるが、
		 * 付け替え中であることに変わりはなく、案内まで消えると操作が分からなくなる
		 * @param isSwapMode 付け替え中ならtrue
		 */
		void setSwapMode(bool isSwapMode) noexcept;

		/**
		 * @brief 能力値の増減表示を消して比較の基準を取り直す
		 *
		 * 開閉をまたいで前回の値を覚えていると、閉じている間に拾ったぶんの変化を
		 * 「今の入れ替えで起きたこと」として見せてしまう。開くたびに呼ぶ
		 */
		void resetStatChanges() noexcept;

		/**
		 * @brief 掴んだものを運んでいる最中かを設定する
		 *
		 * 運んでいる間はアイコンをカーソルへ付いて回らせる。掴んだマスの色だけでは
		 * 「今それを持っている」感じが出ず、どこへ落とすのかも伝わらない
		 * @param isDragging 運んでいる最中ならtrue
		 * @param screenX カーソルのX座標
		 * @param screenY カーソルのY座標
		 */
		void setDragging(bool isDragging, int screenX, int screenY) noexcept;

		/**
		 * @brief 画面座標がどのマスの上にあるかを返す
		 *
		 * マスの位置はレイアウトを組む描画側しか知らないため、当たり判定もここが持つ。
		 * 位置は直前のフレームの描画結果を使うが、1フレームのずれは見えない
		 * @param screenX 画面上のX座標
		 * @param screenY 画面上のY座標
		 * @return 指しているマスの m_acquired 上の位置。どのマスでもなければ -1
		 */
		[[nodiscard]] int findSlotIndexAt(int screenX, int screenY) const noexcept;

		/**
		 * @brief 画面座標が動かせない枠（持ち込み）の上にあるかを返す
		 *
		 * 何も無い場所へ落としたのか、固定された枠へ落とそうとしたのかを分けるために使う。
		 * 前者は取り消し、後者は弾いたことを伝える必要がある
		 * @param screenX 画面上のX座標
		 * @param screenY 画面上のY座標
		 * @return 固定された枠の上ならtrue
		 */
		[[nodiscard]] bool isLockedSlotAt(int screenX, int screenY) const noexcept;

		/// @brief 弾く演出で震わせる対象
		enum class ShakeTarget
		{
			Locked,  // 動かせない枠（持ち込み）すべて
			Selected // 掴んでいるマスと、落とそうとしたマス
		};

		/**
		 * @brief 入れ替えを弾く演出を始める
		 *
		 * 落とせなかったことを、音だけでなく動きでも返す。無反応だと
		 * 操作が効いていないのか、そういう仕様なのかが区別できない
		 * @param target 震わせる対象
		 */
		void startRejectShake(ShakeTarget target) noexcept;

	  private:
		/// @brief 能力値の項目数（左下HUD・セレクト画面と同じ8項目）
		static constexpr int STAT_COUNT{ 8 };

		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief 演出の基準時刻からの経過秒数を返す
		 * @return 経過秒数
		 */
		[[nodiscard]] float elapsedSeconds() const;

		/**
		 * @brief 窓のタイトルバーを描く
		 *
		 * この画面が何なのかを最初に伝える。開いた瞬間に目に入る位置へ置く
		 * @param x 窓左上のX座標
		 * @param y 窓左上のY座標
		 * @param width 窓の幅
		 */
		void drawTitleBar(int x, int y, int width);

		/**
		 * @brief アドレスバー（パンくず）を描く
		 *
		 * エクスプローラーの体裁を作ると同時に、「今どの区分を見ているのか」を
		 * 一言で説明する場所にもなる
		 * @param x 窓左上のX座標
		 * @param y アドレスバー上端のY座標
		 * @param width 窓の幅
		 */
		void drawAddressBar(int x, int y, int width);

		/**
		 * @brief ステータスバー（所持数と閉じ方）を描く
		 * @param x 窓左上のX座標
		 * @param y 窓下端のY座標
		 * @param width 窓の幅
		 * @param itemCount 所持している拡張子の総数
		 */
		void drawStatusBar(int x, int y, int width, int itemCount);

		/**
		 * @brief 所持一覧（未装備）のペインを描く
		 *
		 * 枠数の決まっている区分（持ち込み・装備中）は3つずつしか無く、
		 * 左に積むと右が丸ごと空く。数が決まらない所持一覧をそこへ置き、
		 * 枠で囲って「ここが持ち物の置き場」だと分かるようにする
		 * @param x ペインの左上X座標
		 * @param y ペインの左上Y座標
		 * @param width 使える幅
		 * @param maxBottom これを超える位置には描かない
		 * @param types 並べる拡張子種別（未装備のもの）
		 * @param baseIndex 先頭が m_acquired 上のどの位置にあたるか
		 */
		void drawHoldingPane(int x, int y, int width, int maxBottom,
		    const std::vector<core::data::FileExtensionType>& types, int baseIndex);

		/**
		 * @brief 見出しとマス目の並びを1区分ぶん描く
		 *
		 * 横に並べきれない場合は折り返す。
		 * @param x 区分の左上X座標
		 * @param y 区分の左上Y座標
		 * @param width 使える幅
		 * @param caption 見出し（Shift_JIS変換済み）
		 * @param types 並べる拡張子種別
		 * @param isDimmed 効果が乗っていない扱いで淡く描くか
		 * @param maxBottom これを超える位置には描かない（窓の外へはみ出させない）。
		 *                 収まらなかったぶんは「他 n 件」として件数だけ示す
		 * @param selectableBaseIndex この区分の先頭が m_acquired 上のどの位置にあたるか。
		 *                            -1 なら付け替えの対象外（持ち込みは道中で動かせない）
		 * @param splitBaseCount もともとの枠数。これを超えたぶんは上段へ載せて2段に並べる。
		 *                       0なら分けずに素直に折り返す
		 * @return 描画に使った高さ
		 */
		int drawSection(int x, int y, int width, const std::string& caption,
		    const std::vector<core::data::FileExtensionType>& types, bool isDimmed,
		    int maxBottom, int selectableBaseIndex, int splitBaseCount);

		/// @brief マス目1つの見せ方。旗が増えるたびに引数を足すと呼び出し側が読めなくなる
		struct SlotStyle
		{
			/// @brief 効果が乗っていない扱いで淡く描くか
			bool m_isDimmed{ false };

			/// @brief 選択位置として強調するか
			bool m_isCursor{ false };

			/// @brief 掴んでいる（入れ替え相手を待っている）ものとして強調するか
			bool m_isHeld{ false };

			/// @brief 道中では動かせない枠か（持ち込み）
			bool m_isLocked{ false };

			/// @brief いま弾かれて震えているか
			bool m_isRejected{ false };
		};

		/**
		 * @brief マス目1つ（枠＋アイコン＋ファイル名）を描く
		 * @param x マス左上のX座標
		 * @param y マス左上のY座標
		 * @param type 拡張子種別（Count を渡すと空きマスとして描く）
		 * @param style 見せ方
		 */
		void drawSlot(int x, int y, core::data::FileExtensionType type, const SlotStyle& style);

		/**
		 * @brief 前フレームからの能力値の変化を拾う
		 *
		 * 開いている間は時間が止まっているため、値が動くのは付け替えたときだけ。
		 * 変化を検知すること自体が「入れ替えが起きた」の合図になるので、
		 * イベントを引き回さずに増減を出せる
		 * @param stats このフレームの現在値
		 */
		void trackStatChanges(const utility::PlayerStatValues& stats);

		/**
		 * @brief 増減表示の濃さを返す
		 * @return 不透明度（0なら表示しない）
		 */
		[[nodiscard]] int changeFlashAlpha() const;

		/**
		 * @brief 動かせないマスに付ける南京錠を描く
		 *
		 * 見た目が他のマスと同じだと「ここへ落とせる」と読めてしまう。
		 * 見出しの文字より先に目へ入る記号で、掴もうとする前に伝える
		 * @param x 錠前左上のX座標
		 * @param y 錠前左上のY座標
		 */
		void drawLockBadge(int x, int y);

		/**
		 * @brief 弾く演出の横ずれ量を返す
		 * @return 左右にずらす量（ピクセル。演出中でなければ0）
		 */
		[[nodiscard]] int rejectShakeOffset() const;

		/**
		 * @brief 運んでいる最中のアイコンをカーソルの位置へ描く
		 * @param type 運んでいる拡張子種別（Count なら何も描かない）
		 */
		void drawDraggedIcon(core::data::FileExtensionType type);

		/**
		 * @brief 能力値の一覧を描く
		 * @param x 一覧の左上X座標
		 * @param y 一覧の左上Y座標
		 * @param width 使える幅
		 * @param playerId プレイヤーのEntityID
		 */
		void drawStats(int x, int y, int width, core::ecs::EntityId playerId);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		const data::FileEquipmentData& m_equipmentData;
		HudPanel m_panel;

		/// @brief 付け替えで選べるマスの位置と、それが m_acquired 上のどこかの対応
		struct SlotBounds
		{
			int m_x{ 0 };
			int m_y{ 0 };
			int m_width{ 0 };
			int m_height{ 0 };
			int m_acquiredIndex{ -1 };

			/// @brief 道中では動かせない枠か（このときは m_acquiredIndex を使わない）
			bool m_isLocked{ false };
		};

		// 描画のたびに組み直す。窓の大きさや折り返しが変わっても、
		// 当たり判定を別に計算し直さずに済む
		std::vector<SlotBounds> m_slotBounds{};

		// 周回演出の基準時刻。描画経路からしか呼ばれずdeltaTimeを受け取らないため、
		// 経過時間は壁時計から求める（時間停止中も回り続けてよい演出）
		std::chrono::steady_clock::time_point m_startTime{ std::chrono::steady_clock::now() };

		// 入れ替えの増減表示。前フレームの値と突き合わせて変化を拾い、
		// 拾った時刻から一定時間だけ出す
		utility::PlayerStatValues m_previousStats{};
		utility::PlayerStatValues m_changeAmounts{};
		bool m_hasPreviousStats{ false };
		std::chrono::steady_clock::time_point m_changeTime{};

		// 付け替え操作の状態。位置は m_acquired 上の添字で、-1 は「無し」
		bool m_isSwapMode{ false };
		int m_cursorIndex{ -1 };
		int m_heldIndex{ -1 };

		// 固定枠を弾いた時刻。ここから一定時間だけ左右に震わせる
		std::chrono::steady_clock::time_point m_rejectShakeTime{};
		ShakeTarget m_shakeTarget{ ShakeTarget::Locked };

		// 掴んだものを運んでいる最中か。運んでいる間だけカーソルにアイコンを付ける
		bool m_isDragging{ false };
		int m_dragX{ 0 };
		int m_dragY{ 0 };

		// 拡張子アイコン（FileExtensionTypeの並び順）と空きマスのアイコン
		std::array<int, static_cast<int>(core::data::FileExtensionType::Count)> m_iconHandles{};
		int m_emptyIconHandle{ -1 };

		// 能力値アイコン（左下HUDと同じ並び）
		std::array<int, STAT_COUNT> m_statIconHandles{};

		// DxLibの描画はShift_JISを期待するため、日本語は生成時に一度だけ変換して持つ
		std::string m_title{};
		std::string m_addressText{};
		std::string m_captionCarried{};
		std::string m_captionAcquired{};
		std::string m_captionUnequipped{};
		std::string m_captionNoUnequipped{};
		std::string m_captionStats{};
		std::string m_addressSwapText{};
		std::string m_captionHint{};
		std::string m_captionSwapHint{};
		std::string m_captionEmptySlot{};
		std::string m_captionOverflow{};

		// 拡張子ごとの「何をどれだけ上げるか」の表記（例: "DEF+3"）。
		// extensionBonus.json はプレイ中に変わらないので生成時に組み立てて持つ
		std::array<std::string, static_cast<int>(core::data::FileExtensionType::Count)> m_bonusLabels{};
		std::array<std::string, STAT_COUNT> m_statLabels{};
	};
} // namespace game::ui::ingame
