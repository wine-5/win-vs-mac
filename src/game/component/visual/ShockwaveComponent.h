#pragma once
#include "core/utility/Color.h"
#include "core/utility/Vector3.h"

namespace game::component::visual
{
	/**
	 * @brief 地面を走る衝撃波を出すコンポーネント
	 *
	 * 「踏み鳴らした衝撃が地面を伝わった」ことを見せるためのもの。
	 */
	struct ShockwaveComponent
	{
		// 再生中か。falseの間はSystemが何もしない
		bool m_isPlaying{ false };

		// 再生開始からの経過時間（秒）。Systemが進める
		float m_elapsedTime{ 0.0f };

		// 発生源のワールド座標。startShockwave() が開始時に控える。
		core::Vector3 m_origin{};

		// 輪1本が広がりきるまでの時間（秒）
		float m_duration{ 0.7f };

		// 広がり始めの半径（ワールド単位）。0なら点から広がる。
		// 攻撃で「内側は既に爆発している」表現をしたいときに使う
		float m_startRadius{ 0.0f };

		// 広がりきったときの半径（ワールド単位）
		float m_maxRadius{ 700.0f };

		// 追いかけて出る輪の本数。1本だと単発の波紋にしか見えないので、
		// 少し遅らせて複数出すことで「衝撃が続けて伝わった」ように見せる
		int m_ringCount{ 3 };

		// 輪と輪の間隔（秒）
		float m_ringInterval{ 0.1f };

		// 塗りつぶすか。falseで輪郭のみ。
		// 演出だけなら輪郭、攻撃範囲を示すなら塗りつぶすと危険が伝わりやすい
		bool m_isFilled{ false };

		// 色（0xRRGGBB。濃さは経過時間から算出するのでアルファは含めない）
		unsigned int m_color{ core::utility::Color::rgb(255, 90, 30) };
	};

	/**
	 * @brief 衝撃波を指定位置から再生し始める
	 *
	 * 再生中に呼び直すと最初から出し直す（連続する攻撃で重ならないようにするため）。
	 * 大きさ・色・本数はあらかじめコンポーネントへ設定しておくこと。
	 * @param shockwave 対象のShockwaveComponent
	 * @param origin 衝撃波の中心にするワールド座標（通常は足元）
	 */
	inline void startShockwave(ShockwaveComponent& shockwave, const core::Vector3& origin) noexcept
	{
		shockwave.m_origin = origin;
		shockwave.m_elapsedTime = 0.0f;
		shockwave.m_isPlaying = true;
	}
} // namespace game::component::visual
