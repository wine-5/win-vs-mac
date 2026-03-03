#pragma once
namespace core::iface
{
    /**
     * @brief アニメーション制御の純粋仮想クラス
     * Game層がInfrastructure層に直接依存しないための抽象化
     */
    class IAnimator
    {
    public:
        virtual ~IAnimator() = default;
        virtual void changeAnimation(int modelHandle, int& animIndex, int animHandle, float& totalTime) = 0;
        virtual void updateAnimTime(int modelHandle, int animIndex, float time) = 0;
    };
}