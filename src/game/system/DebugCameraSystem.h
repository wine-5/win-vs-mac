#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/ICamera.h"
#include "core/interface/IInputProvider.h"
#include "core/utility/Vector3.h"

// DEBUG: このファイルはデバッグ用のフリーカメラ機能。リリース時にまとめて削除する。

namespace game::system
{
	/**
	 * @brief デバッグ用のフリーカメラを制御するSystem（マイクラのクリエイティブ視点相当）
	 *
	 * GameManager がデバッグモードのときのみ動作する。マウスで視点を回し、
	 * WASD で水平移動・Space で上昇・Shift で下降する（Playerは矢印キーで別途操作する）。
	 * 通常時（デバッグOFF）は早期リターンし、CameraSystem の通常追従に任せる。
	 */
	class DebugCameraSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief DebugCameraSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param targetEntityId 参照対象（プレイヤー）のEntityID
		 * @param inputProvider 入力のインターフェース
		 * @param camera カメラ装置のインターフェース
		 */
		DebugCameraSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId targetEntityId,
		    core::iface::IInputProvider& inputProvider,
		    core::iface::ICamera& camera);

		/**
		 * @brief デバッグモード時にフリーカメラを更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief デバッグモードに入った瞬間に、通常カメラの位置・向きから初期化する
		 */
		void initializeFromOrbitCamera();

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_targetEntityId{};
		core::iface::IInputProvider& m_inputProvider;
		core::iface::ICamera& m_camera;

		core::Vector3 m_position{ 0.0f, 0.0f, 0.0f }; // フリーカメラのワールド座標
		float m_yaw{ 0.0f };                          // 水平回転（ラジアン）
		float m_pitch{ 0.35f };                       // 垂直回転（ラジアン）
		bool m_wasDebugMode{ false };                 // 前フレームのデバッグモード状態（立ち上がり検出用）
	};
} // namespace game::system
