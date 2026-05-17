#include "WebView2Host.h"
#include <wrl/event.h>

using namespace Microsoft::WRL;

namespace platform::webview
{
    WebView2Host::~WebView2Host() noexcept
    {
        if (m_webview && m_ready)
            m_webview->remove_WebMessageReceived(m_webMessageToken);
        m_webview = nullptr;
        m_controller = nullptr;
    }

    bool WebView2Host::initialize(HWND parentHwnd, const std::wstring& htmlPath) noexcept
    {
        HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this, parentHwnd, htmlPath](
                    HRESULT result, ICoreWebView2Environment* env) -> HRESULT
                {
                    if (FAILED(result)) return result;

                    return env->CreateCoreWebView2Controller(
                        parentHwnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this, parentHwnd, htmlPath](
                                HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                            {
                                if (FAILED(result)) return result;

                                m_controller = controller;

                                ComPtr<ICoreWebView2> webview;
                                controller->get_CoreWebView2(&webview);
                                m_webview = webview;

                                // WebView2 をクライアント領域全体に広げる
                                RECT bounds{};
                                GetClientRect(parentHwnd, &bounds);
                                controller->put_Bounds(bounds);

                                // "game.web" → 実行時 CWD + "\\web" の絶対パスへマッピング
                                // "assets.game.web" → 実行時 CWD + "\\assets" へマッピング
                                ComPtr<ICoreWebView2_3> webview3;
                                if (SUCCEEDED(webview.As(&webview3)))
                                {
                                    wchar_t cwd[MAX_PATH] = {};
                                    GetCurrentDirectoryW(MAX_PATH, cwd);
                                    std::wstring webFolderPath = std::wstring(cwd) + L"\\web";
                                    webview3->SetVirtualHostNameToFolderMapping(
                                        L"game.web", webFolderPath.c_str(),
                                        COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);

                                    std::wstring assetsFolderPath = std::wstring(cwd) + L"\\assets";
                                    webview3->SetVirtualHostNameToFolderMapping(
                                        L"assets.game.web", assetsFolderPath.c_str(),
                                        COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
                                }

                                // JS → C++ メッセージ受信ハンドラ登録
                                webview->add_WebMessageReceived(
                                    Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                        [this](ICoreWebView2*,
                                            ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                                        {
                                            LPWSTR rawMsg = nullptr;
                                            if (SUCCEEDED(args->TryGetWebMessageAsString(&rawMsg))
                                                && rawMsg && m_onMessage)
                                            {
                                                int len = WideCharToMultiByte(
                                                    CP_UTF8, 0, rawMsg, -1,
                                                    nullptr, 0, nullptr, nullptr);
                                                if (len > 0)
                                                {
                                                    std::string utf8(len - 1, '\0');
                                                    WideCharToMultiByte(
                                                        CP_UTF8, 0, rawMsg, -1,
                                                        utf8.data(), len, nullptr, nullptr);
                                                    m_onMessage(utf8);
                                                }
                                                CoTaskMemFree(rawMsg);
                                            }
                                            return S_OK;
                                        }).Get(),
                                    &m_webMessageToken);

                                // ページ読み込み完了後に m_ready = true をセット
                                webview->add_NavigationCompleted(
                                    Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                        [this](ICoreWebView2*,
                                            ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT
                                        {
                                            m_ready = true;
                                            return S_OK;
                                        }).Get(),
                                    &m_navToken);

                                // HTML ページへ移動
                                webview->Navigate(htmlPath.c_str());

                                return S_OK;
                            }).Get()
                    );
                }).Get()
        );

        return SUCCEEDED(hr);
    }

    void WebView2Host::postMessage(const std::wstring& json) noexcept
    {
        if (!m_ready || !m_webview) return;
        m_webview->PostWebMessageAsString(json.c_str());
    }

    void WebView2Host::postMessage(const std::string& utf8Json) noexcept
    {
        if (!m_ready || !m_webview) return;
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8Json.c_str(), -1, nullptr, 0);
        if (len <= 0) return;
        std::wstring wide(len - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8Json.c_str(), -1, wide.data(), len);
        m_webview->PostWebMessageAsString(wide.c_str());
    }

    void WebView2Host::setOnMessage(MessageCallback callback) noexcept
    {
        m_onMessage = std::move(callback);
    }

    void WebView2Host::resize(int width, int height) noexcept
    {
        if (!m_controller) return;
        RECT bounds{0, 0, width, height};
        m_controller->put_Bounds(bounds);
    }

    bool WebView2Host::isReady() const noexcept
    {
        return m_ready;
    }
}
