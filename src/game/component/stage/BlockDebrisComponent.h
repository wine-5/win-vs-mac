#pragma once
#include "core/utility/Vector3.h"
#include <vector>

namespace game::component::stage
{
	/**
	 * @brief 飛び散る破片1つ分の運動状態
	 *
	 * 破片は「割ってあるモデル」のフレームなので、モデルハンドルではなく
	 * フレーム番号で指す。描画はモデル1体ぶんで全破片まとめて行われる
	 */
	struct BlockDebrisFragment
	{
		int m_frameIndex{ -1 };
		core::Vector3 m_pivot{};    // 破片の重心（モデルのローカル座標）
		core::Vector3 m_position{}; // 重心を置くワールド座標
		core::Vector3 m_velocity{};
		core::Vector3 m_rotation{}; // ラジアン
		core::Vector3 m_angular{};  // 回転速度（ラジアン/秒）
		float m_scale{ 1.0f };
	};

	/**
	 * @brief 壊れたブロックの破片が飛散している最中を表すコンポーネント
	 *
	 * 破壊された瞬間に付き、飛散が終わるとEntityごと破棄される。
	 * 無傷のブロックには付けない。壊れるまで待っている大多数のブロックが
	 * 破片ぶんのデータを抱えると、置いただけで無駄なメモリを使うため
	 */
	struct BlockDebrisComponent
	{
		std::vector<BlockDebrisFragment> m_fragments{};

		// 飛散が始まってからの経過時間と、破片が消えるまでの時間（秒）
		float m_elapsed{ 0.0f };
		float m_lifetime{ 1.4f };

		// ブロックのモデルスケール。破片の重心はモデルのローカル座標なので、
		// ワールドの位置へ直すのにこれを掛ける
		float m_modelScale{ 1.0f };

		// 破片が跳ねる床の高さ（ワールドY）。ブロックの底面を使う
		float m_floorY{ 0.0f };
	};
} // namespace game::component::stage
