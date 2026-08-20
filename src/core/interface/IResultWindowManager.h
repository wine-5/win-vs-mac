#pragma once
#include "core/data/ResultData.h"

namespace core::iface
{
    /**
     * @brief リザルト画面のWindow管理のインターフェース
     */
    class IResultWindowManager
    {
    public:
        virtual ~IResultWindowManager() noexcept = default;

        /**
         * @brief ウィンドウを表示し、結果データを反映する
         * @param data リザルトデータ
         */
        virtual void show(const core::data::ResultData& data) noexcept = 0;

        /**
         * @brief メッセージポンプ（毎フレーム呼び出し）
         */
        virtual void pumpMessages() noexcept = 0;

		/**
		 * @brief パッドの操作をWindowのJSへ送る
		 *
		 * リザルト画面は WebView2 のHTMLで、DxLibの入力ループの外にある。
		 * OSのカーソルを合成で動かすと他のアプリへ入力が漏れるため、
		 * 代わりに「何をしたいか」をJSへ渡してフォーカスを動かす
		 * @param action 操作名（"up" / "down" / "left" / "right" / "confirm" / "focus"）
		 */
		virtual void sendPadAction(const char* action) noexcept = 0;
	};
} // namespace core::iface
