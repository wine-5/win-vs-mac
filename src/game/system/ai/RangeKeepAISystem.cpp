#include "RangeKeepAISystem.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include <cmath>
#include <algorithm>

namespace
{
	constexpr float HOVER_RESTORE_SPEED{ 50.0f }; // ホバー高度への復帰速度
	constexpr float STRAFE_RATIO{ 0.85f };        // 横ストレイフ成分の強さ（前後方向の速度に対する倍率）

	// アイドル時のホバー揺らぎ（上下動）：止まっていても「生きている」感を出す
	constexpr float SWAY_AMPLITUDE{ 18.0f }; // 上下に揺れる振幅（ワールド単位）
	constexpr float SWAY_FREQ{ 2.2f };       // 揺れの速さ（rad/秒）
	// 発射時のリコイル：撃った直後に軽く上へ跳ね上がって戻る
	constexpr float RECOIL_KICK{ 90.0f }; // リコイルの押し上げ量係数（m_attackAnimTimer[秒]に掛ける）

	/**
	 * @brief ホバーの目標高度に、アイドル揺らぎと発射リコイルを重ねた値を返す
	 * @param baseHeight 基準のホバー高度
	 * @param elapsedTime 揺らぎの位相計算用の経過時間
	 * @param phase 個体ごとの位相オフセット（群れが同期して揺れないようにする）
	 * @param attackAnimTimer 発射直後の演出タイマー（>0の間リコイルで持ち上がる）
	 * @return 揺らぎ・リコイルを加味した目標高度
	 */
	float hoverTargetHeight(float baseHeight, float elapsedTime, float phase, float attackAnimTimer)
	{
		const float sway{ SWAY_AMPLITUDE * std::sinf(elapsedTime * SWAY_FREQ + phase) };
		const float recoil{ (attackAnimTimer > 0.0f) ? attackAnimTimer * RECOIL_KICK : 0.0f };
		return baseHeight + sway + recoil;
	}
} // namespace

namespace game::system::ai
{
	RangeKeepAISystem::RangeKeepAISystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void RangeKeepAISystem::update(float deltaTime)
	{
		m_elapsedTime += deltaTime; // ホバー揺らぎの位相計算に使う

		// RangeKeepAIComponentを持つ敵だけを処理する
		auto entities{ m_componentManager.getAllEntities<component::ai::RangeKeepAIComponent>() };

		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::AIComponent>(entityId))
				continue;

			// 個体ごとに揺らぎの位相をずらし、複数体が同じタイミングで上下しないようにする
			const float swayPhase{ static_cast<float>(entityId) * 1.7f };

			auto& ai{ m_componentManager.get<component::AIComponent>(entityId) };
			auto& rangeKeep{ m_componentManager.get<component::ai::RangeKeepAIComponent>(entityId) };

			// AIが無効なら処理をスキップ
			if (!ai.m_isActive)
				continue;

			// 追跡対象が設定されていない場合はスキップ
			if (ai.m_targetEntity.getId() == 0)
				continue;

			auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
			auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };

			// ターゲットへの方向ベクトルを計算（3D全軸）
			core::Vector3 direction{};
			direction.x = targetTransform.m_position.x - transform.m_position.x;
			direction.y = targetTransform.m_position.y - transform.m_position.y;
			direction.z = targetTransform.m_position.z - transform.m_position.z;

			float distance{ std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z) };

			// 索敵範囲外なら水平移動・攻撃はしないが、浮遊高度だけは維持する（重力で落とさない）
			if (distance > ai.m_detectionRange)
			{
				if (m_componentManager.has<component::VelocityComponent>(entityId))
				{
					auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
					velocity.m_velocity.x = 0.0f;
					velocity.m_velocity.z = 0.0f;
					// ホバー高度を保つ（指定があれば重力ぶんを補正、なければ垂直速度を0で維持）。
					// 索敵範囲外でもアイドルの上下揺らぎは効かせて「浮いて生きている」感を残す
					if (rangeKeep.m_hoverHeight > 0.0f)
					{
						const float target{ hoverTargetHeight(rangeKeep.m_hoverHeight, m_elapsedTime, swayPhase, rangeKeep.m_attackAnimTimer) };
						const float heightDiff{ target - transform.m_position.y };
						velocity.m_velocity.y = heightDiff * HOVER_RESTORE_SPEED * deltaTime;
					}
					else
						velocity.m_velocity.y = 0.0f;
				}
				continue;
			}

			// 方向ベクトルを正規化
			if (distance > 0.0f)
			{
				direction.x /= distance;
				direction.y /= distance;
				direction.z /= distance;
			}

			// 距離維持ロジック：推奨距離の帯に入るよう前後方向（ラジアル成分）を決める
			core::Vector3 moveDirection{ 0.0f, 0.0f, 0.0f };
			if (distance < rangeKeep.m_preferredDistanceMin)
			{
				// 近すぎたら後退
				moveDirection.x = -direction.x;
				moveDirection.y = -direction.y;
				moveDirection.z = -direction.z;
			}
			else if (distance > rangeKeep.m_preferredDistanceMax)
			{
				// 遠すぎたら接近
				moveDirection.x = direction.x;
				moveDirection.y = direction.y;
				moveDirection.z = direction.z;
			}
			// 推奨距離帯の中では前後成分は0のまま（横のストレイフのみで周回する）

			// 横ストレイフ成分を合成する：プレイヤーを中心に円周を横滑りして回り込む。
			// 水平方向のdirectionを90°回した横ベクトルに、個体ごとの周回向き(+1/-1)を掛ける。
			// 前後（ラジアル）と横（ストレイフ）を合成することで、間合いを保ちつつ回り込む動きになる。
			const float horizontalLen{ std::sqrt(direction.x * direction.x + direction.z * direction.z) };
			if (horizontalLen > 0.0f)
			{
				const float normalizedX{ direction.x / horizontalLen };
				const float normalizedZ{ direction.z / horizontalLen };
				const float strafeSign{ static_cast<float>(rangeKeep.m_strafeDirection) };
				moveDirection.x += -normalizedZ * strafeSign * STRAFE_RATIO;
				moveDirection.z += normalizedX * strafeSign * STRAFE_RATIO;
			}

			// 移動速度を設定（水平と垂直を分ける場合もあるが、ここでは統一）
			if (m_componentManager.has<component::VelocityComponent>(entityId))
			{
				auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
				velocity.m_velocity.x = moveDirection.x * ai.m_moveSpeed;
				velocity.m_velocity.y = moveDirection.y * ai.m_moveSpeed;
				velocity.m_velocity.z = moveDirection.z * ai.m_moveSpeed;
			}

			// ホバー高度を保つ（浮遊敵用）。基準高度にアイドル揺らぎ＋発射リコイルを重ねる
			if (rangeKeep.m_hoverHeight > 0.0f)
			{
				// 目標高度（揺らぎ・リコイル込み）と現在位置の差を垂直速度に反映させる
				// （重力があれば、重力で下がるので、その分を補正）
				const float target{ hoverTargetHeight(rangeKeep.m_hoverHeight, m_elapsedTime, swayPhase, rangeKeep.m_attackAnimTimer) };
				const float heightDiff{ target - transform.m_position.y };
				if (m_componentManager.has<component::VelocityComponent>(entityId))
				{
					auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
					velocity.m_velocity.y += heightDiff * HOVER_RESTORE_SPEED * deltaTime;
				}
			}

			// 向きを更新（プレイヤーの方を向く、水平面のみ）。
			// 機体モデルの正面軸のズレは facingYawOffset で補正する
			const float horizontalDistance{ std::sqrt(direction.x * direction.x + direction.z * direction.z) };
			if (horizontalDistance > 0.0f)
			{
				const float normalizedDirX{ direction.x / horizontalDistance };
				const float normalizedDirZ{ direction.z / horizontalDistance };
				transform.m_rotation.y = std::atan2f(-normalizedDirX, -normalizedDirZ) + rangeKeep.m_facingYawOffset;
			}

			// アニメーション要求：移動中は Walk、停止時は Idle
			if (m_componentManager.has<component::AnimationComponent>(entityId))
			{
				auto& anim{ m_componentManager.get<component::AnimationComponent>(entityId) };
				anim.m_requested = (std::sqrt(moveDirection.x * moveDirection.x + moveDirection.z * moveDirection.z) > 0.01f)
				                       ? constant::AnimationState::Walk
				                       : constant::AnimationState::Idle;
			}

			// 攻撃のクールダウンを更新
			if (ai.m_currentAttackCooldown > 0.0f)
				ai.m_currentAttackCooldown -= deltaTime;

			// 攻撃判定：索敵範囲内かつクールダウンが完了なら攻撃（レンジ判定なし）
			if (m_componentManager.has<component::AttackComponent>(entityId))
			{
				auto& attack{ m_componentManager.get<component::AttackComponent>(entityId) };
				if (ai.m_currentAttackCooldown <= 0.0f)
				{
					attack.m_attackRequested = true;
					ai.m_currentAttackCooldown = ai.m_attackCooldown;
				}
			}
		}
	}
} // namespace game::system::ai
