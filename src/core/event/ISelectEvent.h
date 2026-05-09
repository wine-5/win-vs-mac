#pragma once

namespace core::event
{
	/// @brief Selectシーン用のイベントのマーカーインターフェース
	class ISelectEvent
	{
	public:
		virtual ~ISelectEvent() = default;
	};
}