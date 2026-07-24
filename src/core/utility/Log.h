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
	 * @param fmt 書式文字列
	 * @param args 書式に埋め込む値
	 */
	template <class... Args>
	void warn([[maybe_unused]] std::format_string<Args...> fmt,
	    [[maybe_unused]] Args&&... args)
	{
#ifdef _DEBUG
		base::ServiceLocator::get<iface::ILogger>()->warning(
		    std::format(fmt, std::forward<Args>(args)...).c_str());
#endif
	}

	/**
	 * @brief エラーログを出力する
	 * @param fmt 書式文字列
	 * @param args 書式に埋め込む値
	 */
	template <class... Args>
	void error([[maybe_unused]] std::format_string<Args...> fmt,
	    [[maybe_unused]] Args&&... args)
	{
#ifdef _DEBUG
		base::ServiceLocator::get<iface::ILogger>()->error(
		    std::format(fmt, std::forward<Args>(args)...).c_str());
#endif
	}
} // namespace core::log
