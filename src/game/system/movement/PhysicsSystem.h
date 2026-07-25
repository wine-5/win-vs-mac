#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "game/GameManager.h"

namespace game::system::movement
{
	/**
	 * @brief 速度を元に位置を更新・重力・ジャンプを処理するSystem
	 */
	class PhysicsSystem : public core::ecs::ISystem
	{
	public:
	  /**
	   * @brief PhysicsSystemのコンストラクタ
	   * @param componentManager ComponentManagerの参照
	   * @param gameManager 連続ジャンプ許可フラグを参照するため
	   * @param jumpForce ジャンプの初速（playerData.jsonのgameplay.jumpForce）
	   * @param gravity 重力加速度（負値。playerData.jsonのgameplay.gravity）
	   * @param maxFallSpeed 落下速度の下限（負値。playerData.jsonのgameplay.maxFallSpeed）
	   */
	  PhysicsSystem(core::ecs::ComponentManager& componentManager, GameManager& gameManager, float jumpForce, float gravity, float maxFallSpeed);

	  /**
	   * @brief 速度を元に位置を更新、重力やジャンプを処理する
	   * @param deltaTime フレーム間の時間差
	   */
	  void update(float deltaTime) override;

	private:
	  core::ecs::ComponentManager& m_componentManager;
	  GameManager& m_gameManager;
	  // 重力・落下上限・ジャンプ初速はplayerData.jsonから注入する（値の調整をコード再ビルドなしで行うため）。
	  // ジャンプするのは今のところプレイヤーだけなので、System共通の設定として持つ。
	  // 敵もジャンプするようになったらComponent側へ移す
	  float m_gravity{ -980.0f };
	  float m_jumpForce{ 0.0f };
	  float m_maxFallSpeed{ -200.0f };
	  // 連続ジャンプ無効時、押し続けで浮上しないよう押下の立ち上がりだけを拾うための前フレーム状態。
	  // ジャンプ入力を持つのはプレイヤー1体だけなのでSystem側に1つ持てば足りる
	  bool m_prevJumpPressed{ false };
	};
} // namespace game::system::movement
