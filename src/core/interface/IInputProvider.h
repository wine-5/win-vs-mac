#pragma once
#include "core/input/KeyCode.h"

namespace core::iface
{
	/**
	 * @brief キー入力取得の純粋仮想クラス
	 * Game層がInfrastructure層（DxLib）に直接依存しないための抽象化
	 */
	class IInputProvider
	{
	public:
		virtual ~IInputProvider() = default;
		virtual bool isKeyDown(core::input::KeyCode keycode) const = 0;
	};
}