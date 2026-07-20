#pragma once
#include "core/base/NonCopyable.h"

namespace game
{
	/**
	 * @brief ポーズの理由
	 */
	enum class PauseReason
	{
		None,           // ポーズしていない
		Menu,           // ポーズメニュー（Esc）を開いている
		DebugSceneView, // DEBUG: シーンビュー（F2・時間停止＋フリーカメラ）。リリース時に削除
	};

	/**
	 * @brief ゲーム全体のポーズ状態を管理するクラス
	 * Application が唯一のインスタンスを所有し、シーン間をまたいで生存する。
	 * 利用側へはコンストラクタで参照注入する（Singletonによる暗黙の横断参照を避ける）。
	 */
	class PauseManager : private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief デフォルトコンストラクタ
		 */
		PauseManager() = default;

		/**
		 * @brief 指定した理由でポーズする
		 * @param reason ポーズの理由
		 */
		void pause(PauseReason reason) noexcept
		{
			m_reason = reason;
		}

		/**
		 * @brief ポーズを解除する
		 */
		void resume() noexcept
		{
			m_reason = PauseReason::None;
		}

		/**
		 * @brief 指定した理由でポーズのON/OFFを切り替える
		 * 別の理由でポーズ中の場合は何もしない（例：メニュー中にF2を無効化）
		 * @param reason ポーズの理由
		 */
		void toggle(PauseReason reason) noexcept
		{
			if (m_reason == PauseReason::None)
				m_reason = reason;
			else if (m_reason == reason)
				m_reason = PauseReason::None;
		}

		/**
		 * @brief ポーズ中かどうかを返す
		 * @return ポーズ中ならtrue
		 */
		[[nodiscard]] bool isPaused() const noexcept
		{
			return m_reason != PauseReason::None;
		}

		/**
		 * @brief 指定した理由でポーズ中かどうかを返す
		 * @param reason ポーズの理由
		 * @return その理由でポーズ中ならtrue
		 */
		[[nodiscard]] bool isPausedBy(PauseReason reason) const noexcept
		{
			return m_reason == reason;
		}

		/**
		 * @brief 現在のポーズの理由を返す
		 * @return ポーズの理由（ポーズしていなければNone）
		 */
		[[nodiscard]] PauseReason getReason() const noexcept
		{
			return m_reason;
		}

	  private:
		PauseReason m_reason{ PauseReason::None };
	};
} // namespace game
