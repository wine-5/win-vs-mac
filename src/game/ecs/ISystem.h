#pragma once

namespace game::ecs
{
	// 全てのSystemの純粋仮想クラス
	class ISystem
	{
	public:
		virtual ~ISystem() = default;

		virtual void update(float deltaTime) = 0;
	};
}