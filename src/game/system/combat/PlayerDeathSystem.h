#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "game/constant/AnimationState.h"
#include <vector>

namespace game::system::combat
{
	/**
	 * @brief プレイヤーの死亡演出（死亡アニメ→暗転）を進行させるSystem
	 *
	 * PlayerDeadEventを購読し、以下のタイムラインを駆動する：
	 *   ①死亡アニメ（Dying）再生 → ②余韻の間 → ③画面端から中心へ閉じる暗転 → ④完了イベント発行
	 * 演出中はプレイヤーのInputComponentをロックして操作を無効化する。
	 * 完了時に PlayerDeathSequenceFinishedEvent を発行し、シーン遷移の判断はInGameへ委ねる。
	 * 暗転のdrawはInGameViewの描画フェーズから呼ばれる。
	 */
	class PlayerDeathSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus イベントの購読・発行に使うEventBus
		 * @param uiRenderer UI描画のインターフェース（暗転用）
		 * @param screen 画面サイズ取得のインターフェース
		 * @param playerId 演出対象（プレイヤー）のEntityID
		 */
		PlayerDeathSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 死亡演出のタイムラインを進める
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 暗転（画面端から中心へ閉じるビネット）を描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		/** @brief 死亡演出の進行段階 */
		enum class Phase
		{
			Idle,      // 生存中（演出していない）
			DyingAnim, // 死亡アニメの再生完了待ち
			Hold,      // 倒れたまま見せる余韻
			FadeOut,   // 暗転中
			Finished,  // 完了（イベント発行済み）
		};

		/**
		 * @brief プレイヤーのHPが尽きたときに演出を開始する
		 */
		void onPlayerDead();

		/**
		 * @brief 死亡アニメの再生完了を受け取る
		 * @param entityId 再生が完了したEntityId
		 * @param state 完了したアニメーション状態
		 */
		void onAnimationFinished(core::ecs::EntityId entityId, constant::AnimationState state);

		core::ecs::ComponentManager& m_componentManager;
		core::base::EventBus& m_eventBus;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::EntityId m_playerId;

		Phase m_phase{ Phase::Idle };
		float m_phaseElapsed{ 0.0f }; // 現在の段階に入ってからの経過秒
		float m_fadeProgress{ 0.0f }; // 暗転の進行度（0=通常、1=真っ暗）

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::combat
