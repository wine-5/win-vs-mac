#pragma once

namespace core::ecs
{
	/**
	 * @brief 全てのSystemの基底インターフェース
	 */
	class ISystem
	{
	public:
		virtual ~ISystem() = default;

		virtual void update(float deltaTime) = 0;
	};
}