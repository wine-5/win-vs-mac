#pragma once
#include "core/Vector3.h"
#include "core/interface/IRenderer.h"

namespace infrastructure
{
	/**
	 * @brief 3D描画を担当するクラス
	 */
	class Renderer : public core::iface::IRenderer
	{
	public:
		void drawModel(int modelHandle, const core::Vector3& position, const core::Vector3& rotation, const core::Vector3& scale) override;
		void drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color) override;
	};
}