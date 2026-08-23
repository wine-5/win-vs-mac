#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IInputProvider.h"
#include <string>
#include <vector>

namespace game::system::visual
{
	/**
	 * @brief インゲーム開始時の「ミッション提示 → READY → FIGHT!」演出を担うSystem
	 *
	 * シーンに入った瞬間から戦闘が始まってしまうと、プレイヤーは状況を把握する前に
	 * 敵に詰め寄られる。開始直後に一拍おいて「何をすればいいのか」「これから戦闘が始まる」を
	 * 順に伝えるための演出。
	 *
	 * タイムラインは ①ミッション提示（画面中央・入力があるまで待つ）→ ②左上のObjectiveViewへ
	 * 流れていく → ③READY → ④FIGHT!（この瞬間に操作解禁）→ ⑤FIGHT!のフェードアウトの5段。
	 * ①で入力を待つのは、読み終える速さが人によって違い、固定秒数では誰かに合わないため。
	 * 待たずにいつでも先へ進める（読み終えた人・2周目以降を待たせない）。
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
		 * @param inputProvider 入力取得のインターフェース（ミッション提示を進める操作を読む）
		 * @param playerId 操作ロックの対象（プレイヤー）EntityID
		 */
		BattleStartSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IInputProvider& inputProvider,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 演出タイムラインを進め、解禁のタイミングで操作ロックと敵AIを戻す
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 送る操作が押されたかを読み取り、updateが拾うまで覚えておく
		 *
		 * マウスの押下エッジを内部で更新するため、1フレームに1回だけ呼ぶこと。
		 * updateの中で見ないのは、updateが固定ステップで1フレームに0回のこともあり、
		 * 「押した瞬間」がそのフレームにしか現れないため取りこぼすから
		 */
		void pollAdvanceInput();

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

		/**
		 * @brief 左上の目標表示（ObjectiveView）を出してよい段階かどうかを返す
		 *
		 * ミッションが中央から左上へ流れ着くまでは、同じ内容が2箇所に出ないよう伏せておく
		 * @return 流れ着いていればtrue
		 */
		[[nodiscard]] bool isObjectiveRevealed() const noexcept;

	  private:
		/** @brief 開始演出の進行段階 */
		enum class Phase
		{
			Mission, // ミッションを中央に見せ、入力を待つ
			Fly,     // ミッションが左上のObjectiveViewの位置へ流れていく
			Battle   // READY → FIGHT!（従来の演出）
		};

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
		 * @brief ミッション提示（中央のカード＋一拍おいて出る操作プロンプト）を描画する
		 */
		void drawMission();

		/**
		 * @brief ミッションが中央から左上へ流れていく途中を描画する
		 */
		void drawMissionFly();

		/**
		 * @brief ミッションのカードと文字を、指定の中心・拡大率・濃さで描画する
		 * @param centerX カードの中心X（ピクセル）
		 * @param centerY カードの中心Y（ピクセル）
		 * @param scale 拡大率（1.0で中央表示時の大きさ）
		 * @param alphaRate 濃さ（0.0〜1.0）
		 */
		void drawMissionCard(int centerX, int centerY, float scale, float alphaRate);

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
		core::iface::IInputProvider& m_inputProvider;
		core::ecs::EntityId m_playerId;

		Phase m_phase{ Phase::Mission }; // 現在の段階
		float m_phaseTime{ 0.0f };       // 現在の段階に入ってからの経過時間（秒）
		float m_elapsedTime{ 0.0f };     // READY/FIGHT!のタイムライン上の経過時間（秒）
		bool m_isPlaying{ true };        // 演出中かどうか（構築と同時に始まる）
		bool m_isLocked{ true };         // 操作ロック中かどうか

		// マウス左ボタンは押下エッジを自前で取る（IInputProviderは押下状態しか返さない）。
		// シーンへ入る前のクリックを押しっぱなしと見なして即スキップしないよう、押下状態から始める
		bool m_wasMouseLeftDown{ true };

		// 送る操作が押されたか。フレーム単位で読み取り、updateが拾うまで覚えておく
		bool m_isAdvanceRequested{ false };

		// DxLibの描画はShift_JISを期待するため、ソース上のUTF-8日本語をそのまま渡すと文字化けする。
		// 変換結果は毎フレーム同じなので生成時に一度だけ変換して保持する
		std::string m_missionText{};
		std::string m_missionDetailText{};
		std::string m_promptText{};

		// 演出中に止めた敵のID。解禁時にこれらだけを再開させる（無関係な敵を巻き込まない）
		std::vector<core::ecs::EntityId> m_suspendedEnemyIds{};
	};
} // namespace game::system::visual
