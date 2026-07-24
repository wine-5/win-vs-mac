#pragma once
#include "core/utility/Vector3.h"

namespace game::component::camera
{
	/**
	 * @brief 3人称オービットカメラの状態を持つコンポーネント
	 *
	 * CameraSystem がマウス入力で yaw/pitch を更新し、この値からカメラの位置を計算する。
	 * MoveSystem は m_yaw を読み取り、入力をカメラ相対のワールド移動に変換する。
	 * Shake/Zoom 等の演出も将来ここへフィールドを足して CameraSystem で処理する。
	 */
	struct CameraComponent
	{
		float m_yaw{ 0.0f };            // 水平方向の回転（ラジアン）
		float m_pitch{ 0.35f };         // 垂直方向の回転（ラジアン、上下見上げ/見下ろし）
		float m_distance{ 600.0f };     // 注視点からカメラまでの距離
		float m_targetHeight{ 150.0f }; // 注視点の高さ（プレイヤー足元からの頭あたり）
		float m_fov{ 1.047f };          // 視野角（ラジアン、約60度）
		float m_sensitivity{ 0.003f };  // マウス感度（ラジアン/ピクセル）

		// ピッチの可動範囲（見上げすぎ・見下ろしすぎを防ぐ）
		float m_pitchMin{ -0.3f };
		float m_pitchMax{ 1.2f };

		// カメラの視線方向（単位ベクトル）。CameraSystemが毎フレーム更新する。
		// レティクルの敵捕捉判定・投射の発射方向に流用する。
		core::Vector3 m_forward{ 0.0f, 0.0f, 1.0f };
	};
} // namespace game::component::camera
