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
        
        /**
         * @brief アニメーションを切り替える
         * @param modelHandle モデルハンドル
         * @param animIndex 現在のアニメインデックス（更新される）
         * @param animHandle 新しいアニメハンドル
         * @param totalTime アニメ総再生時間（リセットされる）
         */
        virtual void changeAnimation(int modelHandle, int& animIndex, int animHandle, float& totalTime) = 0;
        
        /**
         * @brief アニメーション再生時間を更新する
         * @param modelHandle モデルハンドル
         * @param animIndex アニメインデックス
         * @param time 再生時間
         */
        virtual void updateAnimTime(int modelHandle, int animIndex, float time) = 0;
    };
}