#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"

namespace game::system
{
	/**
	 * @brief プレイヤー被弾時にカメラを揺らす演出を担うSystem
	 *
	 * AttackHitEventを購読し、被弾者がプレイヤーなら揺れを起動する。
	 * 毎フレーム、残り時間に応じて減衰するランダムな揺れを計算し、
	 * CameraEffectComponentのm_shakeOffset（座標オフセット）だけを書き込むdriver System。
	 * 実際のカメラ合成はCameraSystemが行う。
	 */
	class DamageShakeSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief DamageShakeSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus AttackHitEvent購読用のEventBus
		 * @param playerId プレイヤーのEntityID
		 */
		DamageShakeSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 揺れの残り時間を進め、今フレームの揺れオフセットを計算する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;

		float m_remainingTime{ 0.0f }; // 揺れの残り時間（0以下なら揺れない）
		float m_duration{ 0.0f };      // 揺れ開始時の総時間（減衰カーブ計算用）
		float m_strength{ 0.0f };      // 揺れの振幅（ワールド単位）
		float m_seed{ 0.0f };          // 揺れの位相（起動ごとに変えて毎回違う揺れにする）
	};
} // namespace game::system
