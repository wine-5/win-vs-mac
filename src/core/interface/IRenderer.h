#pragma once
#include "core/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 描画の純粋仮想クラス
	 * Game層がInfrastructre層に直接依存しないための抽象化
	 */
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		virtual void drawModel(int modelHandle, const core::Vector3& position, const::core::Vector3& rotation) = 0;
		virtual void drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color) = 0;
	};
}