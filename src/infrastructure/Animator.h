#pragma once
#include "core/interface/IAnimator.h"

namespace infrastructure
{
	/**
	 * @brief アニメーション制御を担当するクラス
	 */
	class Animator :public core::iface::IAnimator
	{
	public:
		void changeAnimation(int modelHandle, int& animIndex, int animHandle, float& totalTime)override;
		void updateAnimTime(int modelHandle, int animIndex, float time)override;
	};
}