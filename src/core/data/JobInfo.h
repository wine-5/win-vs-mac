#pragma once
#include <string>
#include "core/constant/JobType.h"

namespace core::data
{
	/**
	 * @brief 職業情報をまとめた構造体
	 */
	struct JobInfo
	{
		int m_id{};
		std::string m_name{};
		std::string m_skillName{};
		float m_hp{};
		float m_atk{};
		float m_def{};
		float m_spd{};
	};
}
