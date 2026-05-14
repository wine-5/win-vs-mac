#pragma once

namespace game::data
{
	/**
	 * @brief 職業選択のデータを管理するクラス
	 */
	class JobSelectionData
	{
	public:
		/**
		 * @brief 選択された職業IDを設定
		 * @param jobId 職業ID
		 */
		void setSelectedJobId(int jobId) noexcept
		{
			m_selectedJobId = jobId;
		}

		/**
		 * @brief 選択された職業IDを取得
		 * @return 選択された職業ID（未選択の場合は-1）
		 */
		[[nodiscard]] int getSelectedJobId() const noexcept
		{
			return m_selectedJobId;
		}

		/**
		 * @brief 職業が選択されているかを確認
		 * @return 職業が選択されている場合true
		 */
		[[nodiscard]] bool hasJobSelected() const noexcept
		{
			return m_selectedJobId != -1;
		}

	private:
		int m_selectedJobId{ -1 };
	};
}
