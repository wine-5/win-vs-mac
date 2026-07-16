#include "WindowFactory.h"
#include "loading/LoadingWindow.h"
#include "result/ResultWindow.h"
#include "select/Win32SelectWindowManager.h"
#include "projectile/ProjectileWindowManager.h"
#include "core/interface/IResourceManager.h"

namespace platform::window
{
    WindowFactory::WindowFactory(core::iface::IScreen& screen)
        : m_screen{ screen }
    {
    }

    std::unique_ptr<core::iface::IWindow> WindowFactory::createLoadingWindow(
        std::function<void()> onLoadingComplete)
    {
        // DxLib のクライアント領域を取得
        HWND dxlibHwnd = static_cast<HWND>(m_screen.getNativeWindowHandle());

        RECT clientRect{};
        GetClientRect(dxlibHwnd, &clientRect);
        int screenWidth{ clientRect.right };
        int screenHeight{ clientRect.bottom };

        POINT origin{ 0, 0 };
        ClientToScreen(dxlibHwnd, &origin);
        int originX{ origin.x };
        int originY{ origin.y };

        // ローディングウィンドウを作成
        // スクリーン中央に配置（スクリーンサイズの80%）
        int windowWidth = screenWidth * LOADING_WINDOW_SIZE_RATIO / 100;
        int windowHeight = screenHeight * LOADING_WINDOW_SIZE_RATIO / 100;
        int windowX = originX + (screenWidth - windowWidth) / 2;
        int windowY = originY + (screenHeight - windowHeight) / 2;

        auto loadingWindow = std::make_unique<loading::LoadingWindow>(
            windowX, windowY, windowWidth, windowHeight);

        // ウィンドウを初期化・表示
        loadingWindow->setOnLoadingComplete(std::move(onLoadingComplete));
        if (loadingWindow->create())
            loadingWindow->show();

        return loadingWindow;
    }

    std::unique_ptr<core::iface::IWindow> WindowFactory::createResultWindow(
        std::function<void()> onRetry,
        std::function<void()> onTitle)
    {
        return std::make_unique<result::ResultWindow>(
            m_screen,
            std::move(onRetry),
            std::move(onTitle));
    }

    std::unique_ptr<core::iface::ISelectWindowManager> WindowFactory::createSelectWindowManager(
        std::function<void()> onGameStart,
        std::function<void(core::constant::JobType)> onJobSelect,
        std::function<void(int, const std::string&)> onFileSlotChanged,
        core::iface::IResourceManager& resourceManager)
    {
        return std::make_unique<select::Win32SelectWindowManager>(
            std::move(onGameStart),
            std::move(onJobSelect),
            std::move(onFileSlotChanged),
            resourceManager,
            m_screen);
    }

	std::unique_ptr<core::iface::IProjectileWindowManager> WindowFactory::createProjectileWindowManager()
	{
		HWND gameWindow{ static_cast<HWND>(m_screen.getNativeWindowHandle()) };
		return std::make_unique<ProjectileWindowManager>(gameWindow);
	}
} // namespace platform::window
