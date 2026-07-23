#pragma once
#include "core/ecs/Entity.h"

namespace game::component::combat
{
	/**
	 * @brief 照準（レティクル）の捕捉状態を持つコンポーネント
	 *
	 * TargetingSystemがカメラ前方の敵捕捉を判定して書き込む。
	 * レティクル描画の色分けや、投射の照準対象・発射方向の決定に再利用する。
	 */
	struct AimComponent
	{
		bool m_hasTarget{ false };                                      // カメラ前方に敵を捉えているか
		core::ecs::EntityId m_targetId{ core::ecs::INVALID_ENTITY_ID }; // 捉えている敵のEntityId
	};
} // namespace game::component::combat
