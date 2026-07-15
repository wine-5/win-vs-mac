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

		/**
		 * @brief Systemの更新処理
		 * @param deltaTime フレーム間の時間差
		 */
		virtual void update(float deltaTime) = 0;
	};
} // namespace core::ecs