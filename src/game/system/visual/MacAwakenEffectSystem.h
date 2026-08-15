#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <random>
#include <vector>

namespace game::system::visual
{
	/**
	 * @brief ボスへ寄るシネマ演出を担うSystem（出現時・覚醒時に共用）
	 *
	 * BossAppearedEvent（雑魚全滅→ボス出現）と MacPhaseTransitionEvent（覚醒）を購読し、
	 * どちらも同一のタイムラインを駆動する：
	 *   ①ズームイン（カメラがボスへ寄る）→ ②ホールド（シェイク＋赤ビネット）→ ③ズームアウト → 再開
	 * 毎フレーム、CameraEffectComponentのシネマ・シェイクチャンネルを書き込み（driver System）、
	 * 演出中はプレイヤーのInputComponentをロックして操作を無効化する。
	 * ビネットのdrawはInGameViewの描画フェーズから呼ばれる。
	 */
	class MacAwakenEffectSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus MacPhaseTransitionEvent購読用のEventBus
		 * @param uiRenderer UI描画のインターフェース（赤ビネット用）
		 * @param screen 画面サイズ取得のインターフェース
		 * @param playerId カメラ演出・入力ロックの対象（プレイヤー）EntityID
		 */
		MacAwakenEffectSystem(core::ecs::ComponentManager& componentManager,
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
		/**
		 * @brief 演出中のボスを無敵にする／解除する
		 *
		 * カメラがボスへ寄っている間、プレイヤーは操作を奪われていて狙いを外せない。
		 * その間に飛んでいる弾（雑魚へ撃った流れ弾など）が当たると、
		 * 見ているだけの時間に一方的にダメージが入ってしまうため受け付けない
		 * @param isInvincible 無敵にするならtrue
		 */
		void setMacInvincible(bool isInvincible) noexcept;

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::EntityId m_playerId;

		core::ecs::EntityId m_macId{ core::ecs::INVALID_ENTITY_ID }; // 演出の注視先（イベントで受け取る）

		float m_elapsedTime{ 0.0f };   // 演出開始からの経過時間（秒）
		bool m_isPlaying{ false };     // 演出中かどうか
		float m_vignetteAlpha{ 0.0f }; // 今フレームの赤ビネットの濃さ（0〜1）

		// 今回の演出の強度。トリガー（出現／覚醒）ごとに別プリセットを起動時に取り込む。
		// 出現は控えめ、覚醒は強め、というように個別調整できる
		float m_shakeStrength{ 0.0f };    // ホールド中のシェイクの最大振幅（ワールド単位）
		float m_vignetteStrength{ 0.0f }; // ビネットの最大濃さ（0〜1）

		// ビネットの色。出現はオレンジ、覚醒は赤、と段階を付けて強さの違いを色で示す
		unsigned int m_vignetteColor{ 0u };

		std::mt19937 m_rng{ std::random_device{}() }; // ビネットのちらつき用乱数

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual
