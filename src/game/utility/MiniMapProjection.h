#pragma once
#include "core/utility/Vector3.h"

namespace game::utility
{
	/**
	 * @brief ミニマップ上の位置（マップ中心からのピクセル差分）
	 */
	struct MiniMapPoint
	{
		float m_x{ 0.0f }; // 右が正
		float m_y{ 0.0f }; // 下が正（画面座標に合わせる）
	};

	/**
	 * @brief ワールド座標をミニマップ座標へ変換する
	 *
	 * 回転式（自分の向きが常に上）のミニマップを前提とする。ワールドの+Zを奥＝画面の上へ
	 * 向けるためY成分の符号を反転させる。
	 *
	 * 北固定へ切り替えたくなった場合は、この関数でyawに0を渡せばよい。
	 * 変換をここ1か所に閉じてあるので、向きの方式を変えても呼び出し側は影響を受けない
	 * @param worldX 対象のワールドX座標
	 * @param worldZ 対象のワールドZ座標
	 * @param centerX マップ中心にあたるワールドX座標（自機の位置）
	 * @param centerZ マップ中心にあたるワールドZ座標（自機の位置）
	 * @param yaw 自機の向き（ラジアン）。0を渡すと北固定になる
	 * @param scale ワールド1ユニットあたりのピクセル数
	 * @return マップ中心からのピクセル差分
	 */
	[[nodiscard]] MiniMapPoint projectToMiniMap(float worldX, float worldZ,
	    float centerX, float centerZ, float yaw, float scale) noexcept;

	/**
	 * @brief ワールドのY回転を、ミニマップ上で描くときの回転角へ変換する
	 *
	 * 床の矩形など「向きを持つもの」を描くのに使う。位置の変換（projectToMiniMap）と
	 * 符号を揃えないと、マップ全体の回転と逆向きに自転して見える
	 * @param worldYaw 対象のワールドY回転（ラジアン）
	 * @param yaw 自機の向き（ラジアン）。0を渡すと北固定になる
	 * @return ミニマップ上での回転角（ラジアン・画面座標系）
	 */
	[[nodiscard]] float projectYawToMiniMap(float worldYaw, float yaw) noexcept;
} // namespace game::utility
