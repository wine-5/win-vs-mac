#pragma once
#include "core/interface/IInputProvider.h"

namespace infrastructure
{
	/**
	 * @brief DxLibを使ってキー入力を取得するクラス
	 */
	class InputManager : public core::iface::IInputProvider
	{
	public:
		bool isKeyDown(core::input::KeyCode keyCode) const override;
		bool isPadButtonDown(core::input::GamePadCode code) const override;
		float getPadAxis(core::input::GamePadCode code) const override;
		bool isPadConnected() const override;
	};
}
