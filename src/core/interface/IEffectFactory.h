#pragma once
#include "core/constant/EffectType.h"
#include "core/utility/Vector3.h"

namespace core::iface
{
    /**
     * @brief エフェクトの再生・停止・更新を行うインターフェース
     * Game層がInfra層に依存しないように
     */
    class IEffectFactory
    {
    public:
        virtual ~IEffectFactory() = default;

        /**
         * @brief 起動時にエフェクトプールを初期化する
         */
        virtual void initialize() = 0;

        /**
         * @brief エフェクトを再生する
         * @param type エフェクトの種別
         * @param position 再生位置
         * @return エフェクトの識別ハンドル（stop に使用）
         */
        virtual int play(core::constant::EffectType type, core::Vector3 position) = 0;

        /**
         * @brief エフェクトを停止する
         * @param handle play() が返したハンドル
         */
        virtual void stop(int handle) = 0;

        /**
         * @brief 再生終了したスロットの補充など毎フレームの更新処理
         */
        virtual void update() = 0;

        /**
         * @brief エフェクトが再生中かどうかを返す
         * @param handle play() が返したハンドル
         * @return 再生中なら true
         */
        virtual bool isPlaying(int handle) const = 0;
    };
}