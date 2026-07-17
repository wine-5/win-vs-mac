#pragma once
#include <string>
#include <unordered_map>
#include "core/constant/EffectType.h"
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure::repository
{
    /**
     * @brief Effekseer エフェクトリソースの設定情報
     */
    struct EffectConfig
    {
        int   m_handle  { -1 };   // LoadEffekseerEffect() で取得したハンドル
        int   m_poolSize{ 0 };  // プールの初期サイズ（必ず resources.json から設定する）
        float m_yOffset { 0.0f }; // 再生位置の Y 軸オフセット（必ず resources.json から設定する）
        float m_scale   { 0.0f }; // エフェクトの再生スケール（必ず resources.json から設定する）
    };

    /**
     * @brief Effekseer エフェクトリソースを管理するリポジトリクラス
     *
     * @details resources.json からエフェクトファイルパスと設定を読み込み、
     * LoadEffekseerEffect() でハンドルを取得・キャッシュする
     */
    class EffectRepository
    {
    public:
        EffectRepository() = default;
        ~EffectRepository();

        /**
         * @brief Effekseer 初期化後に呼び出す。resources.json からエフェクトを読み込む
         * @throw std::runtime_error ファイルが見つからないか、JSONパースに失敗した場合
         */
        void initialize();

        /**
         * @brief 全エフェクト設定を返す
         * @return EffectType → EffectConfig のマップ
         */
        [[nodiscard]] const std::unordered_map<core::constant::EffectType, EffectConfig>& getAllConfigs() const;

    private:
        void load(const nlohmann::json& json);

        std::unordered_map<core::constant::EffectType, EffectConfig> m_configs;
    };
} // namespace infrastructure::repository