#pragma once
#include "core/constant/JobType.h"

namespace game::data
{
	/**
	 * @brief 職業選択のデータを管理するクラス
	 */
	class JobSelectionData
	{
	public:
		/**
		 * @brief 選択された職業を設定
		 * @param jobType 職業の種類
		 */
		void setSelectedJobType(core::constant::JobType jobType) noexcept
		{
			m_selectedJobType = jobType;
			m_hasSelection = true;
		}

		/**
		 * @brief 選択された職業を取得
		 * @return 選択された職業の種類
		 */
		[[nodiscard]] core::constant::JobType getSelectedJobType() const noexcept
		{
			return m_selectedJobType;
		}

		/**
		 * @brief 職業が選択されているかを確認
		 * @return 職業が選択されている場合true
		 */
		[[nodiscard]] bool hasJobSelected() const noexcept
		{
			return m_hasSelection;
		}

	private:
		core::constant::JobType m_selectedJobType{ static_cast<core::constant::JobType>(0) };
		bool m_hasSelection{ false };
	};
}
