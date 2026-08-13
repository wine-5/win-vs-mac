#pragma once

namespace game::component::stage
{
	/**
	 * @brief 拡張子を付け替えられる端末であることを表すコンポーネント
	 *
	 * 近づくと足元に案内が出て、F2で付け替え画面を開ける。
	 *
	 * 付け替えをどこでもできるようにすると、敵に会うたびに最適な構成へ
	 * 組み替えるのが正解になり、選択が選択でなくなる。
	 *
	 * @note 壊せない。壊せると付け替えの場そのものが消えてしまう
	 */
	struct RenameTerminalComponent
	{
		// プレイヤーが近づいたと見なす距離（ユニット）。案内はこの距離で出る
		float m_interactRange{ 220.0f };

		// いまプレイヤーが範囲内にいるか。案内の表示と入力の受付に使う
		bool m_isPlayerNear{ false };
	};
} // namespace game::component::stage
