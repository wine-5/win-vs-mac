#pragma once
#include "core/base/NonCopyable.h"

namespace game
{
	/**
	 * @brief ポーズの理由
	 */
	enum class PauseReason
	{
		None,      // ポーズしていない
		Menu,      // ポーズメニュー（Esc）を開いている
		Inventory, // インベントリ（E／△）を開いている（時間は止めない）
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
		 * @brief ポーズ中かどうかを返す
		 * @return ポーズ中ならtrue
		 */
		[[nodiscard]] bool isPaused() const noexcept
		{
			return m_reason != PauseReason::None;
		}

		/**
		 * @brief いまの理由でゲームの時間まで止めるかを返す
		 *
		 * インベントリは開いている間も世界を動かす。付け替えている間に敵が寄ってくることまで
		 * 込みで「倒してから整えるか、そのまま整えるか」を選ばせたいため。
		 * クリアタイムも止めないので、付け替えそのものが時間というコストを持つ。
		 *
		 * ポーズメニューはゲームの一部ではなくシステムの窓（設定・タイトルへ戻る）なので、
		 * こちらは完全に止める
		 * @return 時間を止めるならtrue
		 */
		[[nodiscard]] bool stopsTime() const noexcept
		{
			return m_reason == PauseReason::Menu;
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
