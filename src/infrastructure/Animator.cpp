#include "Animator.h"
#include <DxLib.h>

namespace infrastructure
{
	void Animator::changeAnimation(int modelHandle, int& animIndex, int animHandle, float& totalTime)
	{
		if (animIndex != -1)
			MV1DetachAnim(modelHandle, animIndex);

		// 新しいアニメーションをアタッチ
		animIndex = MV1AttachAnim(modelHandle, 0, animHandle, FALSE);
		totalTime = MV1GetAttachAnimTotalTime(modelHandle, animIndex);
	}

	void Animator::updateAnimTime(int modelHandle, int animIndex, float time)
	{
		MV1SetAttachAnimTime(modelHandle, animIndex, time);
	}
}