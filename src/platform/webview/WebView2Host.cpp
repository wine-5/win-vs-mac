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
                [this, parentHwnd, htmlPath](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
                {
                    if (FAILED(result)) return result;
                    return env->CreateCoreWebView2Controller(
                        parentHwnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this, parentHwnd, htmlPath](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                            {
                                return onControllerCreated(result, controller, parentHwnd, htmlPath);
                            }).Get()
                    );
                }).Get()
        );
        return SUCCEEDED(hr);
    }

    HRESULT WebView2Host::onControllerCreated(HRESULT result, ICoreWebView2Controller* controller,
        HWND parentHwnd, const std::wstring& htmlPath) noexcept
    {
        if (FAILED(result)) return result;

        m_controller = controller;

        ComPtr<ICoreWebView2> webview;
        controller->get_CoreWebView2(&webview);
        m_webview = webview;

        RECT bounds{};
        GetClientRect(parentHwnd, &bounds);
        controller->put_Bounds(bounds);

        ComPtr<ICoreWebView2_3> webview3;
        if (SUCCEEDED(webview.As(&webview3)))
            setupVirtualHostMappings(webview3.Get());

        registerMessageHandler(webview.Get());
        registerNavigationHandler(webview.Get());

        webview->Navigate(htmlPath.c_str());
        return S_OK;
    }

    void WebView2Host::setupVirtualHostMappings(ICoreWebView2_3* webview3) noexcept
    {
        wchar_t cwd[MAX_PATH]{};
        GetCurrentDirectoryW(MAX_PATH, cwd);

        std::wstring base{cwd};
        webview3->SetVirtualHostNameToFolderMapping(
            L"game.web", (base + L"\\web").c_str(),
            COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
        webview3->SetVirtualHostNameToFolderMapping(
            L"assets.game.web", (base + L"\\assets").c_str(),
            COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
    }

    void WebView2Host::registerMessageHandler(ICoreWebView2* webview) noexcept
    {
        webview->add_WebMessageReceived(
            Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                {
                    LPWSTR rawMsg{};
                    if (SUCCEEDED(args->TryGetWebMessageAsString(&rawMsg)) && rawMsg && m_onMessage)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, rawMsg, -1, nullptr, 0, nullptr, nullptr);
                        if (len > 0)
                        {
                            std::string utf8(len - 1, '\0');
                            WideCharToMultiByte(CP_UTF8, 0, rawMsg, -1, utf8.data(), len, nullptr, nullptr);
                            m_onMessage(utf8);
                        }
                        CoTaskMemFree(rawMsg);
                    }
                    return S_OK;
                }).Get(),
            &m_webMessageToken);
    }

    void WebView2Host::registerNavigationHandler(ICoreWebView2* webview) noexcept
    {
        webview->add_NavigationCompleted(
            Callback<ICoreWebView2NavigationCompletedEventHandler>(
                [this](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT
                {
                    m_ready = true;
                    flushPendingMessages();
                    return S_OK;
                }).Get(),
            &m_navToken);
    }

    void WebView2Host::postMessage(const std::wstring& json) noexcept
    {
        if (!m_webview) return;
        if (!m_ready) { m_pendingMessages.push_back(json); return; }
        m_webview->PostWebMessageAsString(json.c_str());
    }

    void WebView2Host::postMessage(const std::string& utf8Json) noexcept
    {
        if (!m_webview) return;
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8Json.c_str(), -1, nullptr, 0);
        if (len <= 0) return;
        std::wstring wide(len - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8Json.c_str(), -1, wide.data(), len);
        if (!m_ready) { m_pendingMessages.push_back(std::move(wide)); return; }
        m_webview->PostWebMessageAsString(wide.c_str());
    }

    void WebView2Host::flushPendingMessages() noexcept
    {
        for (const auto& msg : m_pendingMessages)
            m_webview->PostWebMessageAsString(msg.c_str());
        m_pendingMessages.clear();
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
        if (width > 0 && height > 0)
            m_controller->put_IsVisible(TRUE);
    }

    void WebView2Host::setVisible(bool visible) noexcept
    {
        if (!m_controller) return;
        m_controller->put_IsVisible(visible ? TRUE : FALSE);
    }

    bool WebView2Host::isReady() const noexcept
    {
        return m_ready;
    }
}
