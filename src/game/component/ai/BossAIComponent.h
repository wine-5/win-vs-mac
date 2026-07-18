#pragma once

namespace game::component::ai
{
	/**
	 * @brief ボス型AI用のコンポーネント
	 *
	 * 近接・遠距離を使い分け、複雑な行動パターンを持つボス敵用。
	 * 将来的には HP 比率やフェーズ遷移によって行動が変わるように拡張できる。
	 * BossAISystemが本コンポーネントの有無で「この敵はボス」と判定する。
	 */
	struct BossAIComponent
	{
		// ボスAI用の将来的パラメータスロット
		// 現状は空。状態機械は BossAISystem で実装する。
	};
} // namespace game::component::ai
