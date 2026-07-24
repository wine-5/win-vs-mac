#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

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
	   * @param jumpForce ジャンプの初速（playerData.jsonのgameplay.jumpForce）
	   */
	  PhysicsSystem(core::ecs::ComponentManager& componentManager, float jumpForce);

	  /**
	   * @brief 速度を元に位置を更新、重力やジャンプを処理する
	   * @param deltaTime フレーム間の時間差
	   */
	  void update(float deltaTime) override;

	private:
		static constexpr float DEFAULT_GRAVITY = -98.0f;
		static constexpr float DEFAULT_MAX_FALL_SPEED = -200.0f;

		core::ecs::ComponentManager& m_componentManager;
		float m_gravity{DEFAULT_GRAVITY};
		// ジャンプするのは今のところプレイヤーだけなので、System共通の設定として持つ。
		// 敵もジャンプするようになったらComponent側へ移す
		float m_jumpForce{ 0.0f };
		float m_maxFallSpeed{DEFAULT_MAX_FALL_SPEED};
	};
} // namespace game::system::movement
