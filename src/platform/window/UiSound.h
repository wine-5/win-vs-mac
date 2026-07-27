#pragma once

#include <string>

namespace platform::window
{
	/**
	 * @brief WebViewから届いたJSONが操作音の要求なら鳴らす
	 *
	 * 画面上の操作音は、どのウィンドウでも「押した」「切り替えた」の形が同じなので、
	 * 各ウィンドウのメッセージ処理へ書き写さず、この1か所へ集約する。
	 * 各ウィンドウは自分のハンドラの先頭でこれを呼び、trueなら以降の処理を行わない。
	 *
	 * @param json WebViewから届いたJSON文字列（UTF-8）
	 * @return 操作音の要求として処理した場合true
	 */
	[[nodiscard]] bool tryPlayUiSound(const std::string& json) noexcept;
} // namespace platform::window
