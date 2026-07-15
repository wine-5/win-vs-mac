#pragma once
#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <string>
#include <functional>
#include <vector>

namespace platform::webview
{
    /**
     * @class WebView2Host
     * @brief Win32 ウィンドウのクライアント領域に WebView2 を埋め込むホストクラス
     * @details initialize() を呼ぶと非同期で WebView2 が初期化される。
     *          完了後は postMessage / setOnMessage で C++ ↔ JS 通信できる。
     *          WebView2を初めて使用するというのもありコメントの量が過剰かもしれませんがご了承ください。
     */
    class WebView2Host
    {
    public:
        using MessageCallback = std::function<void(const std::string&)>;

        WebView2Host() noexcept = default;
        ~WebView2Host() noexcept;
        WebView2Host(const WebView2Host&) = delete;
        WebView2Host& operator=(const WebView2Host&) = delete;
        WebView2Host(WebView2Host&&) = delete;
        WebView2Host& operator=(WebView2Host&&) = delete;

        /**
         * @brief WebView2 を初期化し HTML ページをロードする（非同期）
         * @param parentHwnd ホストとなる Win32 ウィンドウハンドル
         * @param htmlPath ロードする URL（例: L"https://game.web/select.html"）
         * @return 初期化リクエストの発行に成功した場合 true
         */
        bool initialize(HWND parentHwnd, const std::wstring& htmlPath) noexcept;

        /**
         * @brief C++ から JS へ JSON 文字列を送信する
         * @param json 送信する JSON 文字列
         */
        void postMessage(const std::wstring& json) noexcept;

        /**
         * @brief C++ から JS へ UTF-8 JSON 文字列を送信する（内部で UTF-16 変換）
         * @param utf8Json 送信する JSON 文字列（UTF-8）
         */
        void postMessage(const std::string& utf8Json) noexcept;

        /**
         * @brief JS から C++ へのメッセージ受信コールバックを設定する
         * @param callback JSON 文字列（UTF-8）を受け取るコールバック
         */
        void setOnMessage(MessageCallback callback) noexcept;

        /**
         * @brief WebView2 のサイズを変更する（WM_SIZE 時に呼ぶ）
         * @param width 新しい幅（ピクセル）
         * @param height 新しい高さ（ピクセル）
         */
        void resize(int width, int height) noexcept;

        /**
         * @brief WebView2 コントローラの表示状態を設定する（WM_SHOWWINDOW 時に呼ぶ）
         * @param visible true なら表示、false なら非表示
         */
        void setVisible(bool visible) noexcept;

        /**
         * @brief WebView2 コントローラが初期化済みか
         * @return 初期化完了なら true
         */
        [[nodiscard]] bool isReady() const noexcept;

    private:
        // 仮想ホストマッピング用定数
        static constexpr const wchar_t* VIRTUAL_HOST_GAME{ L"game.web" };
        static constexpr const wchar_t* VIRTUAL_HOST_ASSETS{ L"assets.game.web" };
        static constexpr const wchar_t* FOLDER_PATH_WEB{ L"\\web" };
        static constexpr const wchar_t* FOLDER_PATH_ASSETS{ L"\\assets" };

        Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller{}; // WebView2の表示領域・サイズ・可視状態を制御するコントローラー
        Microsoft::WRL::ComPtr<ICoreWebView2> m_webview{}; // メッセージ送受信を行うWebViewの本体
        MessageCallback m_onMessage{}; // JS -> C++でメッセージを受信したときに呼ぶコールバック
        bool m_ready{false};
        std::vector<std::wstring> m_pendingMessages{}; // m_readyがfalseの時の間に送信を試みたメッセージを一時的にためておくキュー
        EventRegistrationToken    m_webMessageToken{}; // JSメッセージ受信ハンドラの登録トークンで、デストラクタで解除するために
        EventRegistrationToken    m_navToken{};        // ナビゲーション完了ハンドラの登録トークン

        HRESULT onControllerCreated(HRESULT result, ICoreWebView2Controller* controller,
            HWND parentHwnd, const std::wstring& htmlPath) noexcept;       // コントローラーとWebViewを保存して、サイズ設定・ハンドラ登録・ナビゲーション開始を行う
        void setupVirtualHostMappings(ICoreWebView2_3* webview3) noexcept; // ローカルフォルダを仮想ホスト名に紐づける。HTTPS経由でローカルファイルにアクセスできるようにする
        void registerMessageHandler(ICoreWebView2* webview) noexcept;      // JS -> C++のメッセージ受信イベントハンドラを受信する
        void registerNavigationHandler(ICoreWebView2* webview) noexcept;   // ページ読み込み完了イベントハンドラを登録する。完了時に m_ready = true にし、溜まったメッセージを一括送信する
        void flushPendingMessages() noexcept;                              // m_pendingMessages に溜まっていたメッセージを順番に送信し、キューを空にする
    };
} // namespace platform::webview
