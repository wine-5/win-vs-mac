#pragma once

namespace game::component::ai
{
	/**
	 * @brief 崖から落ちないように歩かせることを示すコンポーネント
	 *
	 * 持っている敵だけが「進む先に床があるか」を確かめてから踏み出す。
	 * 落ちてよい敵（浮遊型など）や、落ちること自体が演出になる敵とは
	 * このコンポーネントの有無で分ける。しきい値はJSONのgameplayから上書きできる。
	 */
	struct CliffAvoidanceComponent
	{
		// 進行方向のどれだけ先の足元を見るか。短すぎると止まりきれず、
		// 長すぎると通れる細道まで避けてしまう
		float m_probeDistance{ 70.0f };

		// 踏み出してよい下りの落差。段差はそのまま降り、これを超える崖には向かわない
		float m_maxStepDown{ 150.0f };

		// 足元より上にある面を「登れる段差」とみなす高さ（GroundingSystemの許容と揃える）
		float m_stepUpTolerance{ 40.0f };
	};
} // namespace game::component::ai
