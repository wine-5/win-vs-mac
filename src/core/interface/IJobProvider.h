#pragma once
#include <string>
#include "core/constant/JobType.h"

namespace core::iface
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

	/**
	 * @brief 職業情報取得のインターフェース
	 */
	class IJobProvider
	{
	public:
		virtual ~IJobProvider() = default;

		/**
		 * @brief 職業数を取得
		 * @return 職業数（通常3）
	 	 */
		virtual int getJobCount() const noexcept = 0;

		/**
		 * @brief 職業情報を取得
		 * @param jobType 職業タイプ
		 * @return 職業情報
		 */
		virtual JobInfo getJobInfo(core::constant::JobType jobType) const = 0;
	};
}