#pragma once

namespace game
{
	/**
	 * @brief ヒットストップ（一瞬の時間停止・スローモーション）を管理するクラス
	 *
	 *  止め方は「InGameがSystemへ渡すdeltaTimeに倍率を掛ける」だけで済む。
	 * このゲームの時間はApplicationが測った実時間をdeltaTimeとして配っているだけで、
	 * 移動もアニメーションもEffekseerも全てその値との掛け算で進むため、
	 * 0を渡せば何も進まない。DxLib側に時間を止める指示は要らない。
	 *
	 * ポーズ（PauseManager）と分けているのは目的も期間も違うため。
	 * ポーズはユーザー操作で任意の長さ止まりUIを重ねる。こちらは0.1秒前後で
	 * 自動的に明け、UIも出さない。今はInGameだけが使うのでInGameが所有する
	 */
	class HitStop
	{
	  public:
		/** @brief クリティカルが出た瞬間に止める */
		void requestOnCritical() noexcept
		{
			request(CRITICAL_DURATION, FULL_STOP_SCALE);
		}

		/** @brief 雑魚を撃破した瞬間に止める */
		void requestOnEnemyKilled() noexcept
		{
			request(ENEMY_KILL_DURATION, FULL_STOP_SCALE);
		}

		/** @brief ボスを撃破した瞬間にスローで見せる */
		void requestOnBossKilled() noexcept
		{
			request(BOSS_KILL_DURATION, BOSS_KILL_SCALE);
		}

		/**
		 * @brief 残り時間を消化し、Systemへ渡すべき時間を返す
		 *
		 * 残り時間は倍率を掛けない元のdeltaTimeで減らす。効果時間は
		 * 「ゲーム内で何秒ぶん進んだか」ではなく実時間で指定するため
		 * @param deltaTime このフレームの本来の時間差（秒）
		 * @return 倍率を掛けた後の時間差。発生していなければdeltaTimeそのまま
		 */
		[[nodiscard]] float apply(float deltaTime) noexcept
		{
			if (m_remainingTime <= 0.0f)
				return deltaTime;

			const float scaled{ deltaTime * m_timeScale };

			m_remainingTime -= deltaTime;
			if (m_remainingTime <= 0.0f)
			{
				m_remainingTime = 0.0f;
				m_timeScale = 1.0f;
			}
			return scaled;
		}

		/**
		 * @brief ヒットストップ中かどうかを返す
		 * @return 効果中ならtrue
		 */
		[[nodiscard]] bool isActive() const noexcept
		{
			return m_remainingTime > 0.0f;
		}

	  private:
		// 場面ごとの効き方。手応えを作るのが目的なので、操作不能に感じない範囲に収める。
		// 呼び出し側は「何が起きたか」だけを伝え、どれだけ止めるかはここで決める
		static constexpr float FULL_STOP_SCALE{ 0.0f };
		// DEBUG: 効きを確かめるため大幅に長くしている。実用値はクリティカル0.07／撃破0.12／ボス0.40
		static constexpr float CRITICAL_DURATION{ 0.30f };
		static constexpr float ENEMY_KILL_DURATION{ 0.60f };
		// ボス撃破だけは決着の区切り。完全停止だと固まって見えるためスローで長めに見せる
		static constexpr float BOSS_KILL_DURATION{ 1.50f };
		static constexpr float BOSS_KILL_SCALE{ 0.15f };

		/**
		 * @brief ヒットストップを開始する
		 *
		 * 発生中に重ねて要求された場合、残り時間は加算せず長いほうを採用する。
		 * ボスのノヴァのように同一フレームで複数体へ当たると、加算では
		 * ヒット数ぶん止まってしまい操作不能に感じるため
		 * @param duration 効果時間（秒）
		 * @param timeScale 効果中の時間の倍率（0.0で完全停止、0.2でスローモーション）
		 */
		void request(float duration, float timeScale) noexcept
		{
			if (duration <= m_remainingTime)
				return;

			m_remainingTime = duration;
			m_timeScale = timeScale;
		}

		float m_remainingTime{ 0.0f };
		float m_timeScale{ 1.0f };
	};
} // namespace game
