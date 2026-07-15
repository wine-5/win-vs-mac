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
		/**
		 * @brief アニメーションを切り替える
		 * @param modelHandle モデルハンドル
		 * @param animIndex 現在のアニメインデックス（更新される）
		 * @param animHandle 新しいアニメハンドル
		 * @param totalTime アニメ総再生時間（リセットされる）
		 */
		void changeAnimation(int modelHandle, int& animIndex, int animHandle, float& totalTime)override;
		
		/**
		 * @brief アニメーション再生時間を更新する
		 * @param modelHandle モデルハンドル
		 * @param animIndex アニメインデックス
		 * @param time 再生時間
		 */
		void updateAnimTime(int modelHandle, int animIndex, float time)override;
	};
} // namespace infrastructure