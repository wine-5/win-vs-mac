#pragma once

namespace game::component
{
	/**
	 * @brief 描画情報を持つコンポーネント
	 */
	struct RenderComponent
	{
		int  m_modelHandle{-1}; // -1は未ロード
		bool m_isVisible{true};
	};
} // namespace game::component