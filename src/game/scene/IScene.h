#pragma once

namespace engine::scene
{
    /**
     * @brief Sceneの基底純粋仮想クラス
     */
    class IScene
    {
    public:
        virtual ~IScene() = default;
        virtual void update(float deltaTime) = 0;
    };
}