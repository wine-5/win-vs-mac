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
	};
}
