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
		/**
		 * @brief Systemを登録する
		 * @tparam T 登録するSystemの型
		 * @tparam Args コンストラクタ引数の型
		 * @param args コンストラクタに渡す引数
		 * @return 登録されたSystemのポインタ
		 */
		template<typename T, typename... Args>
		T* registerSystem(Args&&... args)
		{
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			T* ptr = system.get();
			m_system.push_back(std::move(system));
			return ptr;
		}

		/**
		 * @brief 全てのSystemを更新する
		 * @param deltaTime フレーム間の時間差
		 */
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
