#pragma once

namespace game::component
{
    /**
     * @brief ダメージヒット時の点滅演出を制御するコンポーネント
     *        ヒットエフェクト（パーティクル）は EffectSystem / EffectFactory で管理する
     *        本コンポーネントはモデルの点滅表示のみを担う
     */
    struct HitEffectComponent
    {
        float m_duration{ 1.0f };        // 点滅する合計時間（秒）
        float m_blinkInterval{ 0.1f };   // 表示・非表示をトグルする間隔（秒）
        float m_durationTimer{ 0.0f };   // 点滅の残り時間
        float m_blinkTimer{ 0.0f };      // 次のトグルまでの残り時間
        bool  m_isActive{ false };       // 点滅中かどうか
    };
} // namespace game::component