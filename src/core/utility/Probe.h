#pragma once
#include <string_view>
#include "core/base/ServiceLocator.h"
#include "core/interface/IMemoryProbe.h"

/**
 * @file Probe.h
 * @brief メモリ計測プローブの呼び出し口
 *
 * core/utility/Log.h と同じく ServiceLocator 越しに実装を引くため、
 * Game層・Infrastructure層から Platform層（Windows API）へ直接依存せずに計測できる。
 *
 * 書式を付けたい場合は呼び出し側で std::format した結果を渡す
 * （core::log と違い、ここでは書式化を担わない）。
 *
 * core::log と違い Release ビルドでも動く。調査対象が Release 実行時の使用量のため。
 */
namespace core::probe
{
	/**
	 * @brief 計測地点を記録する
	 *
	 * 直前に mark() を呼んだ地点からの増分を併記するため、切り分けたい処理を
	 * mark() で挟むだけでその処理の確保量が分かる。
	 * プローブが登録されていなければ何もしない。
	 * @param label 計測地点の名前（どの処理の直後かが分かる文字列）
	 */
	inline void mark(std::string_view label)
	{
		if (auto* probe{ base::ServiceLocator::get<iface::IMemoryProbe>() })
			probe->mark(label);
	}

	/**
	 * @brief 計測ログへ任意の1行を追記する（メモリ量を伴わない補足情報用）
	 * @param text 追記する行
	 */
	inline void note(std::string_view text)
	{
		if (auto* probe{ base::ServiceLocator::get<iface::IMemoryProbe>() })
			probe->note(text);
	}
} // namespace core::probe
