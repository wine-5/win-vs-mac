#pragma once

namespace game::component::movement
{
	/**
	 * @brief プレイヤーの入力状態を持つコンポーネント
	 */
	struct InputComponent
	{
		float m_moveX{ 0.0f };
		float m_moveZ{ 0.0f };
		bool  m_jumpPressed{ false };
		bool  m_attackPressed{ false };
		bool m_dashPressed{ false };         // Shift押下でダッシュ
		bool m_rangedAttackPressed{ false }; // 右クリックで遠距離攻撃

		// Tabを押している間だけステータス一覧を開く。インゲームはマウスカーソルを隠すため、
		// 押せる見た目のUIを置かずキーだけで完結させる
		bool m_statusViewPressed{ false };

		// trueの間は全入力を無効化する（ボス覚醒などのシネマ演出中）。演出Systemが書く
		bool m_locked{ false };

		// trueの間は全入力を無効化する（インベントリなどの画面を開いている間）。Sceneが書く。
		// m_lockedと分けているのは持ち主が違うため。同じ変数を2箇所から書くと、
		// 演出中に窓を閉じたときに演出側のロックまで解けてしまう。
		// また敵のAIはm_lockedだけを見る（演出中は狙わないが、窓を開けている間は狙う）
		bool m_uiLocked{ false };

		/**
		 * @brief いま入力を受け付けない状態かを返す
		 * @return 受け付けないならtrue
		 */
		[[nodiscard]] bool isInputBlocked() const noexcept
		{
			return m_locked || m_uiLocked;
		}
	};
} // namespace game::component::movement