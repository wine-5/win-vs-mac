#pragma once
#include <format>
#include <utility>
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace core::log
{
	/**
	 * @brief 通常ログを出力する
	 *
	 * 書式は std::format と同じ "{}" プレースホルダを使う。
	 * 書式と引数の不一致はコンパイルエラーになる（format_string のコンパイル時チェック）。
	 * リリースビルドでは本体ごと消えるため、文字列の組み立てコストも発生しない。
	 * @param fmt 書式文字列
	 * @param args 書式に埋め込む値
	 */
	template <class... Args>
	void info([[maybe_unused]] std::format_string<Args...> fmt,
	    [[maybe_unused]] Args&&... args)
	{
#ifdef _DEBUG
		base::ServiceLocator::get<iface::ILogger>()->log(
		    std::format(fmt, std::forward<Args>(args)...).c_str());
#endif
	}

	/**
	 * @brief 警告ログを出力する
	 *
	 * info と違いリリースビルドでも残す。異常はリリースでこそ起きるうえ、
	 * 全画面表示中はコンソールもダイアログも前面に出せず読めないため、
	 * 後から game_log.txt を開いて追えることが唯一の手掛かりになる
	 * @param fmt 書式文字列
	 * @param args 書式に埋め込む値
	 */
	template <class... Args>
	void warn(std::format_string<Args...> fmt, Args&&... args)
	{
		auto* logger{ base::ServiceLocator::get<iface::ILogger>() };
		if (logger)
			logger->warning(std::format(fmt, std::forward<Args>(args)...).c_str());
	}

	/**
	 * @brief エラーログを出力する
	 *
	 * warn と同じくリリースビルドでも残す
	 * @param fmt 書式文字列
	 * @param args 書式に埋め込む値
	 */
	template <class... Args>
	void error(std::format_string<Args...> fmt, Args&&... args)
	{
		auto* logger{ base::ServiceLocator::get<iface::ILogger>() };
		if (logger)
			logger->error(std::format(fmt, std::forward<Args>(args)...).c_str());
	}
} // namespace core::log
