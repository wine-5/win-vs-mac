#pragma once
#include "core/Vector3.h"

namespace infrastructure
{
	/**
	 * @brief 3D描画を担当するクラス
	 */
	class Renderer
	{
	public:
		void drawModel(int modelHandle, const core::Vector3& position);
	};
}