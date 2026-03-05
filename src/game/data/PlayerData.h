#pragma once
#include <string>
#include "core/Vector3.h"

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
        core::Vector3      getColliderSize() const { return m_colliderSize; }

    private:
        std::string   m_modelPath = "assets/model/Player.mv1";
        // アニメーションのファイルを直したらパスを適用する
        // std::string m_idleAnimPath = "assets/animations/Player_Idle.mv1";
        // std::string m_walkAnimPath = "assets/animations/Player_Walking.mv1";
        std::string   m_idleAnimPath = "None";
        std::string   m_walkAnimPath = "None";
        float         m_moveSpeed = 20.0f;
        core::Vector3 m_colliderSize = { 50.0f, 100.0f, 50.0f };
    };
}