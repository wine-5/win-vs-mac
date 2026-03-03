#pragma once
#include "core/IInputProvider.h"

namespace infrastructure
{
	/**
	 * @brief DxLibを使ってキー入力を取得するクラス
	 */
	class InputManager : public core::IInputProvider
	{
	public:
		bool isKeyDown(core::KeyCode keyCode) const override;
	};
}
