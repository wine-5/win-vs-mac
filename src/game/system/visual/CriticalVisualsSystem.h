#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/base/EventBus.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "game/event/InGameEvents.h"
#include <chrono>
#include <vector>

namespace game::system::visual
{
	/**
	 * @brief クリティカルが出た瞬間に画面の縁から集中線を弾けさせるSystem
	 *
	 * 画面全体を使って一瞬だけ強調する。溜め攻撃の集中線
	 * （PlayerChargeVisualsSystem）と同じ先細りのくさびを使い、
	 * あちらが「力を溜めている」持続の演出なのに対し、こちらは弾けて即消える。
	 *
	 * AttackHitEventを購読し、プレイヤーが敵へ与えたクリティカルにだけ反応する。
	 * 描画はInGameViewのHUD層から呼ばれる
	 */
	class CriticalVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief CriticalVisualsSystemのコンストラクタ
		 * @param componentManager 攻撃側がプレイヤーかを判定するComponentManagerの参照
		 * @param eventBus AttackHitEvent購読用のEventBusの参照
		 * @param uiRenderer 集中線の描画に使うインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param playerId 与えた側がプレイヤーかを判定するためのEntityID
		 */
		CriticalVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::EntityId playerId);

		/**
		 * @brief ISystemの更新（この演出では何もしない）
		 *
		 * 演出の経過時間は壁時計で測るため、ここでは進めない。
		 * クリティカル中はヒットストップでdeltaTimeが数分の1に縮むので、
		 * ゲーム内時間で測ると集中線だけが何倍も長く画面に残ってしまう
		 * @param deltaTime フレーム間の時間差（未使用）
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 集中線を描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		/**
		 * @brief クリティカルを受けて演出を開始する
		 * @param event 攻撃ヒットイベント
		 */
		void onAttackHit(const event::AttackHitEvent& event);

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::EntityId m_playerId{};

		// 演出開始の時刻。描画経路だけで完結させるため壁時計で持つ
		std::chrono::steady_clock::time_point m_startTime{};
		bool m_isActive{ false };

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual
