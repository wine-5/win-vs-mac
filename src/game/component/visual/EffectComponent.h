#pragma once
#include <vector>
#include "core/constant/EffectType.h"

namespace game::component::visual
{
	/**
	 * @brief エフェクト再生状態を管理するコンポーネント
	 * 1つのエンティティに複数のエフェクトを同時再生できる
	 */
	struct EffectComponent
	{
		struct Slot
		{
			core::constant::EffectType m_type{ core::constant::EffectType::None };
			int  m_handle{ -1 };      // IEffectFactory::play()が返すハンドル
		};

		std::vector<Slot> m_slots{};
	};
} // namespace game::component::visual