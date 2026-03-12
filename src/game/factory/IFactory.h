#pragma once

namespace game::factory
{
    /**
     * @brief Factory基底クラス
     * @details オブジェクトの生成と寿命管理を担当する
     *
     * ## 実装規約
     * - 派生クラスは必ず `create()` メソッドを実装すること
     * - 本来であればcreate関数を定義したいが、引数が異なるためコメントのみを記載
     */
    class IFactory
    {
    public:
        virtual ~IFactory() = default;
    };
}
