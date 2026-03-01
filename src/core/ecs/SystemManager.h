#pragma once
#include <vector>
#include <memory>
#include "ISystem.h"

namespace core::ecs
{
	/**
	 * @brief 全てのSystemを登録・更新するマネージャクラス
	 */
	class SystemManager
	{
	public:
		template<typename T, typename... Args>
		T* registerSystem(Args&&... args)
		{
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			T* ptr = system.get();
			m_system.push_back(std::move(system));
			return ptr;
		}

		void update(float deltaTime)
		{
			for (auto& system : m_system)
			{
				system->update(deltaTime);
			}
		}

	private:
		// 登録順に更新されるSystemのリスト
		std::vector<std::unique_ptr<ISystem>> m_system;
	};
}
