#pragma once

namespace core::data
{
	/**
	 * @brief 音量の設定値
	 *
	 * 値は 0〜100 の段階で持つ。スライダーの表示も保存ファイルの中身もこの段階値で、
	 * 実際に音へ掛ける倍率へは toGain() で変換する。内部を最初から小数にすると
	 * 「50 と表示しているのに 0.5 ではない」というずれが起きるため、正は段階値に置く
	 */
	struct AudioSettings
	{
		/** @brief スライダーの右端 */
		static constexpr int MAX_LEVEL{ 100 };
		/** @brief 増減の刻み幅（ゲームパッドの左右でも扱える粒度にする） */
		static constexpr int LEVEL_STEP{ 5 };

		int m_master{ MAX_LEVEL };
		int m_bgm{ MAX_LEVEL };
		int m_se{ MAX_LEVEL };

		/**
		 * @brief 段階値を音量の倍率へ変換する
		 *
		 * 人の耳は音量を対数で感じるため、段階値をそのまま倍率にすると
		 * 50 まで下げても半分に聞こえない。2乗を掛けて聴感に近づける。
		 * 100 のときは 1.0 になるので、既定値のままなら従来の音量と変わらない
		 * @param level 0〜100 の段階値
		 * @return 音へ掛ける倍率（0.0〜1.0）
		 */
		[[nodiscard]] static constexpr float toGain(int level) noexcept
		{
			const float normalized{ static_cast<float>(level) / MAX_LEVEL };
			return normalized * normalized;
		}

		/**
		 * @brief BGM に掛ける倍率を返す（マスターとの積）
		 * @return 音へ掛ける倍率（0.0〜1.0）
		 */
		[[nodiscard]] constexpr float bgmGain() const noexcept
		{
			return toGain(m_master) * toGain(m_bgm);
		}

		/**
		 * @brief SE に掛ける倍率を返す（マスターとの積）
		 * @return 音へ掛ける倍率（0.0〜1.0）
		 */
		[[nodiscard]] constexpr float seGain() const noexcept
		{
			return toGain(m_master) * toGain(m_se);
		}
	};

	/**
	 * @brief 操作・カメラの設定値
	 */
	struct ControlSettings
	{
		/** @brief カメラ感度の下限（Windowsのポインター速度と同じく段階で持つ） */
		static constexpr int MIN_SENSITIVITY{ 1 };
		/** @brief カメラ感度の上限 */
		static constexpr int MAX_SENSITIVITY{ 10 };
		/** @brief カメラ感度の既定値（この段階のとき従来と同じ速さになる） */
		static constexpr int DEFAULT_SENSITIVITY{ 5 };
		/** @brief 感度が既定値のときのラジアン/ピクセル */
		static constexpr float BASE_SENSITIVITY{ 0.003f };

		/** @brief 画面の揺れの最大値（100 で従来どおりの揺れ） */
		static constexpr int MAX_SHAKE{ 100 };
		/** @brief 画面の揺れの刻み幅 */
		static constexpr int SHAKE_STEP{ 10 };

		int m_sensitivity{ DEFAULT_SENSITIVITY };
		bool m_invertY{ false };
		int m_screenShake{ MAX_SHAKE };

		/**
		 * @brief 段階値をカメラ感度（ラジアン/ピクセル）へ変換する
		 * @return マウス移動量へ掛ける係数
		 */
		[[nodiscard]] constexpr float sensitivityPerPixel() const noexcept
		{
			return BASE_SENSITIVITY * static_cast<float>(m_sensitivity) / DEFAULT_SENSITIVITY;
		}

		/**
		 * @brief 縦方向のマウス移動へ掛ける符号を返す
		 *
		 * 横は反転させない。左右まで逆になると単に操作できなくなるだけで、
		 * 反転を求める人が欲しいのは縦だけのため
		 * @return 反転しているなら -1.0、そうでなければ 1.0
		 */
		[[nodiscard]] constexpr float pitchDirection() const noexcept
		{
			return m_invertY ? -1.0f : 1.0f;
		}

		/**
		 * @brief 画面の揺れに掛ける倍率を返す
		 * @return 揺れ幅への倍率（0.0〜1.0、0.0 で揺れなし）
		 */
		[[nodiscard]] constexpr float shakeScale() const noexcept
		{
			return static_cast<float>(m_screenShake) / MAX_SHAKE;
		}
	};

	/**
	 * @brief プレイヤーが変更できる設定の全体
	 *
	 * 保存ファイル（settings.json）の中身と1対1で対応する。
	 * 項目を増やすときはここへ構造体を足し、SettingsRepository の読み書きを合わせる
	 */
	struct GameSettings
	{
		AudioSettings m_audio{};
		ControlSettings m_control{};
	};
} // namespace core::data
