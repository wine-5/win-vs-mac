#pragma once
#include <cmath>

namespace game::ui::ingame
{
	/**
	 * @brief 低HP警告の点滅を計算する
	 *
	 * HPバーと画面端のビネットを同じリズムで脈打たせるため、計算をここへ一本化する。
	 * 別々に持つと、片方の定数だけを触ったときに点滅がずれて心拍に見えなくなる
	 */
	namespace low_health
	{
		constexpr float THRESHOLD{ 0.2f };    // これ以下の残量比で警告を出す
		constexpr float PERIOD_SLOW{ 1.20f }; // 警告が始まった直後の1周期（秒）
		constexpr float PERIOD_FAST{ 0.55f }; // HPが尽きる寸前の1周期（秒）
		constexpr float TWO_PI{ 6.283185f };

		/**
		 * @brief 警告を出す残量かどうかを返す
		 * @param ratio HPの残量比（0.0〜1.0）
		 * @return 警告を出すならtrue
		 */
		[[nodiscard]] constexpr bool isLow(float ratio) noexcept
		{
			return ratio <= THRESHOLD;
		}

		/**
		 * @brief 切迫度を返す
		 * @param ratio HPの残量比（0.0〜1.0）
		 * @return 警告が始まった時点で0.0、HPが尽きる時点で1.0
		 */
		[[nodiscard]] constexpr float computeDanger(float ratio) noexcept
		{
			if (ratio >= THRESHOLD)
				return 0.0f;
			if (ratio <= 0.0f)
				return 1.0f;
			return 1.0f - ratio / THRESHOLD;
		}

		/**
		 * @brief 点滅の明るさを返す
		 *
		 * 切迫しているほど速く点滅させ、危険の度合いを速さでも伝える
		 * @param ratio HPの残量比（0.0〜1.0）
		 * @param elapsedSeconds 基準時刻からの経過秒数
		 * @return 0.0（消灯）〜1.0（最も明るい）
		 */
		[[nodiscard]] inline float computeWave(float ratio, float elapsedSeconds) noexcept
		{
			const float period{ PERIOD_SLOW + (PERIOD_FAST - PERIOD_SLOW) * computeDanger(ratio) };
			// 0.0から始めて往復させる。加算合成では0のとき何も足されず、元の色に戻る
			return 0.5f - 0.5f * std::cos(elapsedSeconds / period * TWO_PI);
		}
	} // namespace low_health
} // namespace game::ui::ingame
