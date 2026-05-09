#pragma once

namespace core::event
{

	/**
	 * @brief InGameシーン用のイベントのマーカーインターフェース
	 */
	class IGameEvent
	{
	public:
		virtual ~IGameEvent() = default;
	};
}