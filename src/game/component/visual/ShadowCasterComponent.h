#pragma once

namespace game::component::visual
{
	/**
	 * @brief 足元へ影を落とす対象であることを示すコンポーネント
	 *
	 * 付けたEntityだけが影を落とす。プレイヤー・敵・拾える欠片など種類が増えても
	 * 描画側は変えずに「付けるかどうか」だけで切り替えられるようにしている。
	 * 一時的に影を消したい場合は RenderComponent の m_isVisible を落とす。
	 *
	 * @note DxLibのシャドウマップ（MakeShadowMap系）は本作では使っていない。
	 *       影を落とす側は正しく焼けている（TestDrawShadowMapでシルエットを確認済み）が、
	 *       SetUseShadowMap が成功を返すのに受け側へ一切反映されなかったため、
	 *       確実に出る接地影で置き換えている。詳細は docs/next_tasks.md を参照。
	 */
	struct ShadowCasterComponent
	{
		// 影の半径。0ならColliderComponentの幅から自動で決める。
		// 実際の投影ではなく見た目で決める量なので、個別に詰めたいものだけ指定する
		float m_radius{ 0.0f };

		// 直近で接地していた高さ。ジャンプ中も影を地面へ残すために覚えておく
		float m_groundY{ 0.0f };

		// 一度でも接地して m_groundY が埋まったか。埋まるまでは影を描かない
		bool m_hasGroundY{ false };
	};
} // namespace game::component::visual
