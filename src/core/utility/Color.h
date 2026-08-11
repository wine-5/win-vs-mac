#pragma once

namespace core::utility
{
    /**
     * @brief 色を扱うユーティリティクラス
     */
    class Color
    {
    public:
        /**
         * @brief RGB値から色を作成する（Alpha=255）
         * @param r 赤成分（0-255）
         * @param g 緑成分（0-255）
         * @param b 青成分（0-255）
         * @return ARGB形式の色（0xAARRGGBB）
         */
        static constexpr unsigned int rgb(int r, int g, int b)
        {
            return 0xFF000000 | (r << 16) | (g << 8) | b;
        }

        /**
         * @brief ARGB値から色を作成する
         * @param a アルファ成分（0-255、255=不透明）
         * @param r 赤成分（0-255）
         * @param g 緑成分（0-255）
         * @param b 青成分（0-255）
         * @return ARGB形式の色（0xAARRGGBB）
         */
        static constexpr unsigned int argb(int a, int r, int g, int b)
        {
            return (a << 24) | (r << 16) | (g << 8) | b;
        }

        // ========== 基本色 ==========

        static constexpr unsigned int WHITE = 0xFFFFFFFF;
        static constexpr unsigned int BLACK = 0xFF000000;
        static constexpr unsigned int RED = 0xFFFF0000;
        static constexpr unsigned int GREEN = 0xFF00FF00;
        static constexpr unsigned int BLUE = 0xFF0000FF;
        static constexpr unsigned int YELLOW = 0xFFFFFF00;
        static constexpr unsigned int CYAN = 0xFF00FFFF;
        static constexpr unsigned int MAGENTA = 0xFFFF00FF;
        static constexpr unsigned int GRAY = 0xFF808080;
        static constexpr unsigned int DARK_GRAY = 0xFF404040;
        static constexpr unsigned int LIGHT_GRAY = 0xFFC0C0C0;
        static constexpr unsigned int DARK_BLUE = 0xFF0000C8;
        static constexpr unsigned int MEDIUM_GREEN = 0xFF00B400;
		static constexpr unsigned int WINDOWS_LOGO_BLUE = 0xFF00ADEF; // Windowsロゴの水色

		// ========== UI用の色 ==========

        // ボタンの色
        static constexpr unsigned int BUTTON_NORMAL    = 0xFF4848A0;
        static constexpr unsigned int BUTTON_FOCUSED   = 0xFF6868D0;
        static constexpr unsigned int BUTTON_PRESSED   = 0xFF383880;
        static constexpr unsigned int BUTTON_DISABLED  = GRAY;

        // ========== パフォーマンスグラフ用の色 ==========

        static constexpr unsigned int GRAPH_CPU    = 0xFF4080FF; // 青：CPU使用率
        static constexpr unsigned int GRAPH_MEMORY = 0xFF40FF80; // 緑：メモリ使用率
        static constexpr unsigned int GRAPH_DISK   = 0xFF80FFFF; // シアン：ディスク活動率
        static constexpr unsigned int CARD_BG      = 0xFF081024; // タスクマネージャー風カード背景（濃い紺）

		// ========== 発見演出（通知バナー）用の色 ==========

		static constexpr unsigned int ALERT_BANNER_BG = 0xFFF8F8FA;    // 通知バー背景の代替色（画像未ロード時）
		static constexpr unsigned int ALERT_DANGER_RED = 0xFFE02424;   // 危険メッセージの赤
		static constexpr unsigned int ALERT_SUBTEXT_GRAY = 0xFF8A8A90; // 敵の種類名（アプリ名）の薄いグレー

		// ========== 攻撃予兆（テレグラフ）用の色（ARGB。アルファで半透明度を指定） ==========

		// 予兆は敵の攻撃にだけ出すため、危険を示す赤オレンジ系で固定する
		static constexpr unsigned int TELEGRAPH_BASE = 0x40FF5028; // 危険範囲の下地（薄いオレンジ赤）
		static constexpr unsigned int TELEGRAPH_FILL = 0x78FF6030; // 満ちていく内側（オレンジ）
		static constexpr unsigned int TELEGRAPH_RING = 0xC8FF3820; // 外周リング（濃い赤オレンジ）

		// ========== InGame HUD（Windows 11 / Fluent）用の色 ==========

		static constexpr unsigned int HUD_INK = 0xFFEAF1FB;         // 文字・レティクルの基本色（わずかに青みのある白）
		static constexpr unsigned int HUD_CHARGE_CYAN = 0xFF22D3EE; // 溜め中のシアン
		// 溜め最大の黄色。ステージが青〜シアン基調のため、白では背景に溶けて完了が分からない
		static constexpr unsigned int HUD_CHARGE_MAX = 0xFFFFC83D;
		// クリティカル（クリティカル）のオレンジ。溜め完了の黄色より赤に寄せて役割を分ける。
		// シアン＝プレイヤーの技、オレンジ＝クリティカル、赤＝危険、で色の意味を重複させない
		static constexpr unsigned int HUD_CRITICAL_ORANGE = 0xFFFF7A18;
		static constexpr unsigned int HUD_CRIT_RED = 0xFFE81123;    // 敵を捕捉中・危険
		static constexpr unsigned int HUD_ACCENT = 0xFF0078D4;      // Windows 11のアクセント色
		static constexpr unsigned int HUD_INK_FAINT = 0xFF5E708A;   // 補足情報・未装備などの控えめな文字
		// 能力が上がったことを示す緑。強化中の黄色（HUD_CHARGE_MAX）と役割が違う。
		// 黄色は「今この能力は強化された状態」という継続した状態、
		// こちらは「たった今上がった」という瞬間を表す
		static constexpr unsigned int HUD_BUFF_GREEN = 0xFF4ADE80;
		// HPが十分あるときのバーの緑。上がったことを示す HUD_BUFF_GREEN より暗くしてある。
		// バーは塗る面積が広く、同じ明るさだと画面の中でいちばん目立つ色になってしまう
		static constexpr unsigned int HUD_BAR_GREEN = 0xFF36D07B;

		// HUDのパネル・マス目の面と枠。窓・スロット・吹き出し・バーの溝まで
		// すべて同じ2色で組み、画面ごとに濃さが違って見えないようにする
		static constexpr unsigned int HUD_PANEL_FILL = 0xFF0E1420;   // 面（濃紺）
		static constexpr unsigned int HUD_PANEL_BORDER = 0xFF8CAAD2; // 枠（淡い青灰）

		// 道中では動かせないものを示す赤。危険を表す HUD_CRIT_RED より彩度を落としてある。
		// 出しっぱなしにする色なので、警告と同じ強さだと視界を占領してしまう
		static constexpr unsigned int HUD_LOCKED_RED = 0xFFC0524F;

		// ========== メニュー（ポーズなどの項目リスト）用の色 ==========

		// 選択中の項目。非選択の薄いグレーと色相・明度の両方で離し、
		// 色を見分けにくい環境でもどれを選んでいるか分かるようにする
		static constexpr unsigned int MENU_SELECTED_GOLD = 0xFFFFD700;
		static constexpr unsigned int MENU_UNSELECTED_GRAY = 0xFFC8C8C8;
	};
} // namespace core::utility