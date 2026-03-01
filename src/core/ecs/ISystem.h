#pragma once

namespace game::ecs
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