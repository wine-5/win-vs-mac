#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <random>

namespace game::system
{
	/**
	 * @brief ボス覚醒（フェーズ移行）時のシネマ演出を担うSystem
	 *
	 * BossPhaseTransitionEventを購読し、以下のタイムラインを駆動する：
	 *   ①ズームイン（カメラがボスへ寄る）→ ②ホールド（シェイク＋赤ビネット）→ ③ズームアウト → 再開
	 * 毎フレーム、CameraEffectComponentのシネマ・シェイクチャンネルを書き込み（driver System）、
	 * 演出中はプレイヤーのInputComponentをロックして操作を無効化する。
	 * ビネットのdrawはInGameViewの描画フェーズから呼ばれる。
	 */
	class BossAwakenEffectSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus BossPhaseTransitionEvent購読用のEventBus
		 * @param uiRenderer UI描画のインターフェース（赤ビネット用）
		 * @param screen 画面サイズ取得のインターフェース
		 * @param playerId カメラ演出・入力ロックの対象（プレイヤー）EntityID
		 */
		BossAwakenEffectSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 演出タイムラインを進め、カメラ演出チャンネル・入力ロックを更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 赤ビネットを描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::EntityId m_playerId;

		core::ecs::EntityId m_bossId{ core::ecs::INVALID_ENTITY_ID }; // 演出の注視先（イベントで受け取る）

		float m_elapsedTime{ 0.0f };   // 演出開始からの経過時間（秒）
		bool m_isPlaying{ false };     // 演出中かどうか
		float m_vignetteAlpha{ 0.0f }; // 今フレームの赤ビネットの濃さ（0〜1）

		std::mt19937 m_rng{ std::random_device{}() }; // ビネットのちらつき用乱数
	};
} // namespace game::system
