#pragma once
#include "core/event/ISelectEvent.h"
#include <string>

namespace core::event
{
	/**
	 * @brief 職業選択が変更されたときに発行されるイベント
	 */
	struct JobChangedEvent : public ISelectEvent
	{
		/** @brief 選択された職業のID */
		int m_jobId{ -1 }; // 初期値は無効値

		JobChangedEvent() = default;
		JobChangedEvent(int jobId) : m_jobId(jobId) {}
	};

	/**
	 * @brief ファイルスロットが選択/変更されたときに発行されるイベント
	 */
	struct FileSlotChangedEvent : public ISelectEvent
	{
		/** @brief 選択されたスロットのインデックス */
		int m_slotIndex{ -1 };

		/** @brief 選択されたファイルパス */
		std::string m_filePath{};

		FileSlotChangedEvent() = default;
		FileSlotChangedEvent(int slotIndex, const std::string& filePath)
			: m_slotIndex(slotIndex)
			, m_filePath(filePath)
		{
		}
	};

	/**
	 * @brief ゲーム開始がリクエストされたときに発行されるイベント
	 */
	struct GameStartRequestedEvent : public ISelectEvent
	{
		GameStartRequestedEvent() = default;
	};
}