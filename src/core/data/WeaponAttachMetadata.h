#pragma once
#include <string>
#include "core/utility/Vector3.h"

namespace core::data
{
	/**
	 * @brief 武器をキャラクターのボーンへ装着する設定（JSONの weapon 要素に対応）
	 *
	 * 握りの位置・角度・見た目の長さは実機で見ないと決まらないため、
	 * 再ビルドせず調整できるようJSONで持つ。
	 */
	struct WeaponAttachMetadata
	{
		std::string modelId{};   // 装着するモデルのID（resources.jsonのrawModels）
		std::string frameName{}; // 装着先のボーン名（mv1変換後の名前）

		core::Vector3 offsetPosition{}; // ボーンのローカル空間での位置ずらし
		core::Vector3 offsetRotation{}; // ボーンのローカル空間での回転（度。game層でラジアンへ変換）

		// 見た目の全長（ワールド単位）。モデル実寸から必要な拡大率を逆算するのに使う。
		// 0以下ならモデルの実寸そのまま（拡大率1倍）
		float length{ 0.0f };
	};
} // namespace core::data
