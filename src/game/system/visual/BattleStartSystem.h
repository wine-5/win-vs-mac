#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <vector>

namespace game::system::visual
{
	/**
	 * @brief インゲーム開始時の「READY → FIGHT!」演出を担うSystem
	 *
	 * シーンに入った瞬間から戦闘が始まってしまうと、プレイヤーは状況を把握する前に
	 * 敵に詰め寄られる。開始直後に一拍おいて「これから戦闘が始まる」と宣言するための演出。
	 *
	 * タイムラインは ①READY（操作ロック・敵AI停止）→ ②FIGHT!（この瞬間に操作解禁）
	 * → ③FIGHT!のフェードアウト（この間はもう動ける）の3段。
	 * 文字の描画はInGameViewの描画フェーズから呼ばれる。
	 */
	class BattleStartSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ（この時点で演出を開始し、操作と敵AIを止める）
		 * @param componentManager ComponentManagerの参照
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param playerId 操作ロックの対象（プレイヤー）EntityID
		 */
		BattleStartSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 演出タイムラインを進め、解禁のタイミングで操作ロックと敵AIを戻す
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief READY / FIGHT! の文字を描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

		/**
		 * @brief 開始前の待機中（操作がロックされている）かどうかを返す
		 *
		 * 経過タイムの計測開始を FIGHT! の瞬間まで遅らせるために使う
		 * @return READY表示中ならtrue
		 */
		[[nodiscard]] bool isPreparing() const noexcept;

	  private:
		/**
		 * @brief 1080p基準で書いた寸法を、実際の画面高さに合わせて拡縮する
		 * @param value 1080p基準のピクセル数
		 * @return 現在の画面での ピクセル数
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief プレイヤーの操作ロックと敵AIの停止を切り替える
		 * @param isLocked 止めるならtrue
		 */
		void setGameplayLocked(bool isLocked);

		/**
		 * @brief READYの文字（フェードイン→ホールド→フェードアウト）を描画する
		 */
		void drawReady();

		/**
		 * @brief FIGHT!の文字（発光する横帯＋画面フラッシュ）を描画する
		 */
		void drawFight();

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::EntityId m_playerId;

		float m_elapsedTime{ 0.0f }; // 演出開始からの経過時間（秒）
		bool m_isPlaying{ true };    // 演出中かどうか（構築と同時に始まる）
		bool m_isLocked{ true };     // 操作ロック中かどうか

		// 演出中に止めた敵のID。解禁時にこれらだけを再開させる（無関係な敵を巻き込まない）
		std::vector<core::ecs::EntityId> m_suspendedEnemyIds{};
	};
} // namespace game::system::visual
