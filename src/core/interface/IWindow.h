#pragma once

namespace core::iface
{
    /**
     * @brief ウィンドウの基本インターフェース
     * @details Platform層のウィンドウがこのインターフェースを実装することで、
     *          Game層がPlatform依存部分に依存しないようにする
     */
    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        /**
         * @brief メッセージポンプ（毎フレーム呼び出し）
         */
        virtual void pumpMessages() noexcept = 0;

        /**
         * @brief ウィンドウを破棄する
         */
        virtual void destroy() noexcept = 0;
    };
} // namespace core::iface
