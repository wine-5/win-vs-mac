#pragma once

namespace game::component::visual
{
	/**
	 * @brief 影を落とす対象であることを示すコンポーネント
	 *
	 * 付けたEntityだけがシャドウマップへ描かれる。プレイヤー・敵・拾える欠片など、
	 * 種類が増えても描画側は変えずに「付けるかどうか」だけで切り替えられるようにしている。
	 * 一時的に影を消したい場合は RenderComponent の m_isVisible を落とす。
	 *
	 * 影を「受ける」側の指定は不要で、通常どおり描いた物すべてに影が落ちる。
	 * 床・壁のように受けるだけの物へ付けると、二度描きのぶん重くなるだけで絵は変わらない。
	 *
	 * @note 受ける側のメッシュで背面カリングを無効（DX_CULLING_NONE）にすると、
	 *       DxLibはそのメッシュへ影を一切適用しない。配置物のカリングを切らないようにする。
	 */
	struct ShadowCasterComponent
	{
	};
} // namespace game::component::visual
