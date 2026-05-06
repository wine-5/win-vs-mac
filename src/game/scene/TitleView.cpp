#include "TitleView.h"
#include "game/ui/Button.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"

namespace game::scene
{
	TitleView::TitleView(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen)
		: m_inputProvider{ inputProvider }
		, m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
	{
		setupUI();
	}

	
}