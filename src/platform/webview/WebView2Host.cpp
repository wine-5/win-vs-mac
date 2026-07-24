#include "WebView2Host.h"
#include <wrl/event.h>

using namespace Microsoft::WRL;

namespace platform::webview
{
    WebView2Host::~WebView2Host() noexcept
    {
		// ハンドラは初期化直後（ページ読み込み完了より前）に登録されるため、
		// m_ready ではなくWebView本体の有無で解除を判断する。
		// m_ready を条件にすると、読み込み完了前に破棄されたときに解除漏れになる
		if (m_webview)
		{
			m_webview->remove_WebMessageReceived(m_webMessageToken);
			m_webview->remove_NavigationCompleted(m_navToken);
		}

		// WebView2 の推奨手順に従い、コントローラーを明示的に閉じてから解放する
		if (m_controller)
			m_controller->Close();

		// ComPtr のリリース順：WebView 本体 → コントローラーの順で解放する
        m_webview = nullptr;
        m_controller = nullptr;
    }

    bool WebView2Host::initialize(HWND parentHwnd, const std::wstring& htmlPath) noexcept
    {
        // WebView2の実行環境の生成を非同期で依頼する
        HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr, // Edgeのパスなし、ユーザーデータのフォルダなし、環境オプションなし
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this, parentHwnd, htmlPath](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
                {
                    if (FAILED(result)) return result;
                    return env->CreateCoreWebView2Controller(
                        parentHwnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this, parentHwnd, htmlPath](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                            {
                                // ウィンドウの中にWebView2を表示するように依頼
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

        // コントローラーと WebView 本体を保存する
        m_controller = controller;

        ComPtr<ICoreWebView2> webview;
        controller->get_CoreWebView2(&webview);
        m_webview = webview;

        // WebView の表示領域をクライアント全体に合わせる
        RECT bounds{};
        GetClientRect(parentHwnd, &bounds);
        controller->put_Bounds(bounds);

        // ICoreWebView2_3 にキャストできれば仮想ホストマッピングを設定する
        // （古いランタイムではキャスト失敗の可能性があるため SUCCEEDED でガードしている）
        ComPtr<ICoreWebView2_3> webview3;
        if (SUCCEEDED(webview.As(&webview3)))
            setupVirtualHostMappings(webview3.Get());

        registerMessageHandler(webview.Get());
        registerNavigationHandler(webview.Get());

        // ハンドラ登録後にナビゲーションを開始する
        webview->Navigate(htmlPath.c_str());
        return S_OK;
    }

    void WebView2Host::setupVirtualHostMappings(ICoreWebView2_3* webview3) noexcept
    {
        // 実行ディレクトリを基準にローカルフォルダを仮想ホスト名に紐づける。
        // これにより JS 側から https:// スキームでローカルファイルを参照できる
        wchar_t cwd[MAX_PATH]{};
        GetCurrentDirectoryW(MAX_PATH, cwd);

        std::wstring base{cwd};
        // https://game.web/... → <cwd>/web/ 以下のファイルを返す
        webview3->SetVirtualHostNameToFolderMapping(
            VIRTUAL_HOST_GAME, (base + FOLDER_PATH_WEB).c_str(),
            COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
        // https://assets.game.web/... → <cwd>/assets/ 以下のファイルを返す
        webview3->SetVirtualHostNameToFolderMapping(
            VIRTUAL_HOST_ASSETS, (base + FOLDER_PATH_ASSETS).c_str(),
            COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
    }

    void WebView2Host::registerMessageHandler(ICoreWebView2* webview) noexcept
    {
        webview->add_WebMessageReceived(
            Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                {
                    LPWSTR rawMsg{};
                    // TryGetWebMessageAsString で文字列として取得する（JSON ではなく文字列として送っているため）
                    if (SUCCEEDED(args->TryGetWebMessageAsString(&rawMsg)) && rawMsg && m_onMessage)
                    {
                        // WebView2 から届く文字列は UTF-16 なので UTF-8 に変換してコールバックへ渡す
                        int len = WideCharToMultiByte(CP_UTF8, 0, rawMsg, -1, nullptr, 0, nullptr, nullptr);
                        if (len > 0)
                        {
                            std::string utf8(len - 1, '\0');
                            WideCharToMultiByte(CP_UTF8, 0, rawMsg, -1, utf8.data(), len, nullptr, nullptr);
                            m_onMessage(utf8);
                        }
                        // rawMsg は COM タスクメモリで確保されているため CoTaskMemFree で解放する
                        CoTaskMemFree(rawMsg);
                    }
                    return S_OK;
                }).Get(),
            &m_webMessageToken);
    }

    void WebView2Host::registerNavigationHandler(ICoreWebView2* webview) noexcept
    {
        // ページ読み込みが完了したら準備完了フラグを立て、待機中のメッセージを一括送信する
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
        // WebView2 の初期化が完了していない場合はキューに積んで後で一括送信する
        if (!m_ready) { m_pendingMessages.push_back(json); return; }
        m_webview->PostWebMessageAsString(json.c_str());
    }

    void WebView2Host::postMessage(const std::string& utf8Json) noexcept
    {
        if (!m_webview) return;
        // UTF-8 → UTF-16 に変換してから送信する
        int len{ MultiByteToWideChar(CP_UTF8, 0, utf8Json.c_str(), -1, nullptr, 0) };
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
        // サイズが 0 のとき（最小化など）は表示しない。有効なサイズになったら自動で再表示する
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
} // namespace platform::webview
