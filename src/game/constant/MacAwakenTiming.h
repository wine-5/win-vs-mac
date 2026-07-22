#pragma once

namespace game::constant::mac_awaken
{
	/**
	 * @brief ボス覚醒演出のタイムライン（秒）
	 *
	 * 演出を出す MacAwakenEffectSystem と、その間ボスを停止（ロック）させる
	 * MacAISystem の両方で共有する。各フェーズの時間をここで定義し、
	 * 合計（TOTAL_TIME）は導出する（ベタ書きの合計値による二重管理を避ける）。
	 */
	constexpr float ZOOM_IN_TIME{ 1.5f };  // ①カメラがボスへ寄る
	constexpr float HOLD_TIME{ 2.0f };     // ②寄ったまま：シェイク＋赤ビネット
	constexpr float ZOOM_OUT_TIME{ 1.0f }; // ③カメラを引いて通常へ戻す

	// 演出全体の長さ。MacAISystemのフェーズ移行ロック時間と一致する
	constexpr float TOTAL_TIME{ ZOOM_IN_TIME + HOLD_TIME + ZOOM_OUT_TIME };
} // namespace game::constant::mac_awaken
