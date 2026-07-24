#pragma once

namespace game::component::visual
{
	/**
	 * @brief 描画情報を持つコンポーネント
	 */
	struct RenderComponent
	{
		int m_modelHandle{ -1 }; // -1は未ロード
		bool m_isVisible{ true };

		// テクスチャの繰り返し回数。1.0なら引き伸ばし（従来どおり）。
		// 引き伸ばした配置物で模様が間延びしないよう、実寸に応じて繰り返す
		float m_uvScaleU{ 1.0f };
		float m_uvScaleV{ 1.0f };

		// テクスチャを流す速さ（1.0でテクスチャ1枚ぶん/秒）。0なら流れない
		float m_scrollSpeedU{ 0.0f };
		float m_scrollSpeedV{ 0.0f };

		// 現在のずらし量。TextureScrollSystemが時間で進める
		float m_scrollOffsetU{ 0.0f };
		float m_scrollOffsetV{ 0.0f };
	};
} // namespace game::component::visual