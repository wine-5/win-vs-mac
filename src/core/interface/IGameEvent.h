#pragma once

namespace core::iface
{

	/**
	 * @brief InGameシーン用のイベントのマーカーインターフェース
	 */
	class IGameEvent
	{
	public:
		virtual ~IGameEvent() = default;
	};
} // namespace core::iface