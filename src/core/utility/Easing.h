#pragma once
#include <algorithm>

namespace core::utility
{
	/**
	 * @file Easing.h
	 * @brief 進行度（0〜1）に緩急を付ける関数
	 *
	 * 等速のまま動かすと機械的に見えるため、演出はどこかで緩急を付けることになる。
	 * 引数は0〜1へ丸めるため、呼び出し側で範囲を気にしなくてよい。
	 */

	/**
	 * @brief 加速しながら0→1へ（落下・沈み込み）
	 *
	 * 動き始めが遅く、終わりが速い。重力で落ちる動きに合う。
	 * @param t 進行度（0〜1。範囲外は丸める）
	 * @return 補間値（0〜1）
	 */
	[[nodiscard]] constexpr float easeIn(float t) noexcept
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return t * t;
	}

	/**
	 * @brief 減速しながら0→1へ（飛び出し・跳ね上がり）
	 *
	 * 動き始めが速く、終わりで緩む。勢いよく出て減衰する動きに合う。
	 * @param t 進行度（0〜1。範囲外は丸める）
	 * @return 補間値（0〜1）
	 */
	[[nodiscard]] constexpr float easeOut(float t) noexcept
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return 1.0f - (1.0f - t) * (1.0f - t);
	}

	/**
	 * @brief 両端が緩やかな0→1へ（smoothstep）
	 *
	 * 始まりと終わりの両方で緩む。カメラの寄りや文字のフェードのように
	 * 「上品に見せたい」動きに合う。
	 * @param t 進行度（0〜1。範囲外は丸める）
	 * @return 補間値（0〜1）
	 */
	[[nodiscard]] constexpr float smoothstep(float t) noexcept
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}
} // namespace core::utility
