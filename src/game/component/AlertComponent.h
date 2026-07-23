#pragma once

namespace game::component
{
	/**
	 * @brief 発見演出（通知バッジ）の表示状態を持つコンポーネント
	 *
	 * EnemyAlertedEvent受信時にDetectionAlertSystemが付与し、タイマーが尽きたら外す。
	 * タイマーはバッジの出現バウンド・フェードアウトの進行にも使う
	 */
	struct AlertComponent
	{
		float m_timer{ 0.0f };   // バナー表示の残り時間（秒）
		int m_messageIndex{ 0 }; // 表示する危険メッセージの番号（発見時に抽選し、表示中は固定）
	};
} // namespace game::component
