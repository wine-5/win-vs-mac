#pragma once
#include <string>

namespace game::data
{
	/**
	 * @brief Playerのデータを保持するクラス
	 */
	class PlayerData
	{
	public:
		const std::string& getModelPath()    const { return m_modelPath; }
		const std::string& getIdleAnimPath() const { return m_idleAnimPath; }
		const std::string& getWalkAnimPath() const { return m_walkAnimPath; }
		float              getMoveSpeed()    const { return m_moveSpeed; }

	private:
		std::string m_modelPath = "assets/model/Player.mv1";
		std::string m_idleAnimPath = "assets/animation/Player_Idle.mv1";
		std::string m_walkAnimPath = "assets/animation/Player_Walking.mv1";
		float       m_moveSpeed = 5.0f;
	};
}