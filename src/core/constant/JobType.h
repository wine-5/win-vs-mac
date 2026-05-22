#pragma once

namespace core::constant
{
	/**
	 * @brief 職業の型列挙型
	 */
	enum class JobType
	{
		Warrior = 0,  // 剣士
		Mage = 1,     // 魔法使い
		Ninja = 2     // 忍者
	};

	/// @brief 職業の総数
	constexpr int JOB_COUNT = 3;
}