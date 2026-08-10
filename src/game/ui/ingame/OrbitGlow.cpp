#include "OrbitGlow.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include <algorithm>
#include <cmath>

namespace
{
	// 縁を周回する光の粒
	constexpr float PERIOD{ 3.2f };          // 一周にかける秒数
	constexpr int COMET_COUNT{ 2 };          // 同時に回る粒の列の数（外周上で等間隔に配置する）
	constexpr int TRAIL_COUNT{ 16 };         // 1列あたりの粒の数（後ろほど淡くなる）
	constexpr float TRAIL_SPACING{ 0.011f }; // 粒どうしの間隔（周回全体を1.0とした割合）
	constexpr int ALPHA{ 210 };              // 加算合成の強さ（粒ごとの明暗は色側で付ける）

	/**
	 * @brief 色の明るさを倍率で落とす
	 *
	 * 加算合成では色を暗くすることが透明度を下げることと同じ意味になる。
	 * 粒ごとにブレンドモードを設定し直さずに済ませるため、明暗は色側で付ける
	 * @param color 元の色（ARGB形式：0xAARRGGBB）
	 * @param scale 明るさの倍率（0.0〜1.0）
	 * @return 暗くした色
	 */
	unsigned int scaleBrightness(unsigned int color, float scale)
	{
		auto channel = [&](int shift)
		{ return static_cast<int>(((color >> shift) & 0xFFu) * scale); };
		return core::utility::Color::argb(255, channel(16), channel(8), channel(0));
	}

	/**
	 * @brief 矩形の外周上の点を求める
	 *
	 * 進行度は辺の長さに比例して配分する。4辺へ均等に割ると、
	 * 縦長の矩形では短い辺だけ粒が速く走って見える。
	 * 角丸ぶんのズレは半径が小さく、粒が角を通る一瞬しか出ないため無視する
	 * @param x 左上のX座標
	 * @param y 左上のY座標
	 * @param width 幅
	 * @param height 高さ
	 * @param t 外周をひと回りする進行度（0.0〜1.0。0.0が左上で時計回り）
	 * @param outX 求めたX座標の格納先
	 * @param outY 求めたY座標の格納先
	 */
	void pointOnRectPerimeter(int x, int y, int width, int height, float t, int& outX, int& outY)
	{
		const float wrapped{ t - std::floor(t) };
		const float perimeter{ static_cast<float>(width + height) * 2.0f };
		float along{ wrapped * perimeter };

		if (along < width) // 上辺（左→右）
		{
			outX = x + static_cast<int>(along);
			outY = y;
			return;
		}
		along -= width;

		if (along < height) // 右辺（上→下）
		{
			outX = x + width;
			outY = y + static_cast<int>(along);
			return;
		}
		along -= height;

		if (along < width) // 下辺（右→左）
		{
			outX = x + width - static_cast<int>(along);
			outY = y + height;
			return;
		}
		along -= width;

		outX = x; // 左辺（下→上）
		outY = y + height - static_cast<int>(along);
	}
} // namespace

namespace game::ui::ingame::orbit_glow
{
	void draw(core::iface::IUIRenderer& uiRenderer, int x, int y, int width, int height,
	    float elapsedSeconds, float phaseOffset, int dotRadius)
	{
		const float head{ elapsedSeconds / PERIOD + phaseOffset };
		const int radius{ std::max(2, dotRadius) };

		// 加算合成で重ねると、粒が枠線の上を通るときに芯が白く抜けて発光して見える。
		// 粒ごとの明暗はアルファではなく色で付けるため、ブレンドの設定は1回で済む
		uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, ALPHA);

		for (int comet{ 0 }; comet < COMET_COUNT; ++comet)
		{
			// 列を外周上で等間隔に散らす（2列なら向かい合う位置になる）
			const float cometHead{ head + static_cast<float>(comet) / COMET_COUNT };

			for (int i{ 0 }; i < TRAIL_COUNT; ++i)
			{
				// 後続ほど過去の位置に置き、暗く小さくして尾を引かせる
				const float fade{ 1.0f - static_cast<float>(i) / TRAIL_COUNT };

				int dotX{ 0 };
				int dotY{ 0 };
				pointOnRectPerimeter(x, y, width, height, cometHead - i * TRAIL_SPACING, dotX, dotY);

				uiRenderer.drawCircle(dotX, dotY, std::max(1, static_cast<int>(radius * fade)),
				    scaleBrightness(core::utility::Color::HUD_CHARGE_CYAN, fade * fade), true, 1);
			}
		}

		uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame::orbit_glow
