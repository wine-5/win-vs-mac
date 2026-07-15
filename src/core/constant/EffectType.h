#pragma once

namespace core::constant
{
	/**
	 * @brief エフェクトの種類を表す列挙型
	 */
	enum class EffectType
	{
		None,
		Hit,  // ヒット
		
		// 今後攻撃のエフェクトなど追加予定
		// EffectRepositoryのtypeMapにも追加を忘れないように
	};
} // namespace core::constant