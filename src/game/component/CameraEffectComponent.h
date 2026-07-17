#pragma once
#include "core/utility/Vector3.h"

namespace game::component
{
	/**
	 * @brief カメラ演出の合成状態を持つコンポーネント（Blackboard）
	 *
	 * 各演出のdriver System（ChargeZoomSystem・DamageShakeSystemなど）が
	 * 自分の担当チャンネルだけを書き込み、CameraSystemが全チャンネルを読んで
	 * 基本カメラに合成する。演出同士は別チャンネルなので互いに干渉しない。
	 *
	 * 新しい演出を足すときは、ここにチャンネルを1つ増やし、
	 * それを書くdriver Systemを1つ追加する（既存の演出には触れない）。
	 */
	struct CameraEffectComponent
	{
		// ズーム：FOVへの倍率（1.0＝通常、<1.0＝望遠で寄る、>1.0＝広角）。ChargeZoomSystemが書く。
		float m_fovScale{ 1.0f };

		// ルーズ（引き）：カメラ距離への倍率（1.0＝通常、>1.0＝引く）。将来のdriver Systemが書く。
		float m_distanceScale{ 1.0f };

		// シェイク：今フレームのカメラ位置・注視点への揺れオフセット。DamageShakeSystemが書く。
		core::Vector3 m_shakeOffset{ 0.0f, 0.0f, 0.0f };
	};
} // namespace game::component
