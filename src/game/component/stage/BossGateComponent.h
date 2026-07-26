#pragma once

namespace game::component::stage
{
	/**
	 * @brief ボス出現で入り口を塞ぐ扉（シャッター）のコンポーネント
	 *
	 * ステージJSONに置かれた位置を「閉じた状態」とし、開いている間は床下へ沈めておく。
	 * BossGateSystemがボス出現イベントを受けて閉位置までせり上げる。
	 * 当たり判定はStagePropのBoxコライダーがTransformに追従するので、
	 * せり上がりきればそのまま通せんぼになる（判定側に専用処理は要らない）。
	 */
	struct BossGateComponent
	{
		// 閉じきったときのY（＝エディタで置いた高さ）
		float m_closedY{ 0.0f };

		// 開いているときのY（床下へ沈めた高さ）
		float m_openY{ 0.0f };

		// 閉じ具合。0で開ききり、1で閉じきり
		float m_progress{ 0.0f };

		// 閉じ動作中か。ボス出現イベントでtrueになる
		bool m_isClosing{ false };
	};
} // namespace game::component::stage
