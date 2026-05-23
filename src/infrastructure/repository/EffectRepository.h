#pragma once
#include <string>
#include <unordered_map>
#include "core/constant/EffectType.h"
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure
{
    /**
     * @brief Effekseer エフェクトリソースを管理するリポジトリクラス
     *
     * @details resources.json からエフェクトファイルパスを読み込み、
     * LoadEffekseerEffect() でハンドルを取得・キャッシュする
     */
    class EffectRepository
    {
    public:
        /**
         * @brief コンストラクタ
         *
         * コンストラクト時に resources.json からエフェクト情報を読み込む
         * @throw std::runtime_error ファイルが見つからないか、JSONパースに失敗した場合
         */
        EffectRepository();

        /**
         * @brief EffectType に対応するリソースハンドルを返す
         * @param type エフェクト種類
         * @return LoadEffekseerEffect() のハンドル（未登録の場合は -1）
         */
        [[nodiscard]] int getHandle(core::constant::EffectType type) const;

    private:
        void load(const nlohmann::json& json);

        std::unordered_map<core::constant::EffectType, int> m_handles;
    };
}