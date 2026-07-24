#pragma once
#include "core/utility/Vector3.h"
#include "core/interface/ICamera.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLibのカメラを操作する装置クラス
	 * 位置・注視点・FOVを設定するだけの薄い層。yaw/pitch等の計算はCameraSystemが行う。
	 */
	class Camera : public core::iface::ICamera
	{
	public:
	  /**
	   * @brief カメラの位置と注視点を設定する
	   * @param position カメラのワールド座標
	   * @param target 注視点のワールド座標
	   */
	  void setLookAt(const core::Vector3& position, const core::Vector3& target) override;

	  /**
	   * @brief 視野角（FOV）を設定する（将来のズーム演出用）
	   * @param fovRad 視野角（ラジアン）
	   */
	  void setFieldOfView(float fovRad) override;
	};
} // namespace infrastructure::graphics
