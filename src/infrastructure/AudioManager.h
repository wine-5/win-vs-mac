#pragma once
#include "core/interface/IAudioManager.h"
#include "infrastructure/repository/AudioRepository.h"

namespace infrastructure
{
	/**
	 * @brief IAudioManager の実装クラス
	 * BGM はフェードイン・アウトに対応し、SE は即時再生する
	 */
	class AudioManager : public core::iface::IAudioManager
	{
	public:
		~AudioManager();

		/**
		 * @brief サウンドリソースを読み込み、各ハンドルを初期化する
		 * @throw std::runtime_error ファイルが見つからないか、JSON パースに失敗した場合
		 */
		void initialize() override;

		/**
		 * @brief BGM を再生する
		 * @param type 再生する BGM の種別
		 * @param fade true の場合フェードイン・アウトを使用する（デフォルト: true）
		 */
		void playBgm(core::constant::BgmType type, bool fade = true) override;

		/**
		 * @brief 現在再生中の BGM を停止する
		 * @param fade true の場合フェードアウトを使用する（デフォルト: true）
		 */
		void stopBgm(bool fade = true) override;

		/**
		 * @brief SE を再生する
		 * @param type 再生する SE の種別
		 */
		void playSe(core::constant::SeType type) override;

		/**
		 * @brief フェード処理など毎フレームの更新処理
		 */
		void update() override;

	private:
		/** @brief フェード処理の種類 */
		enum class FadeState
		{
			None,
			FadeIn,
			FadeOut,
		};

		/** @brief フェード速度（1フレームあたりの音量変化量、0.0f〜1.0f） */
		static constexpr float FADE_SPEED{ 0.01f };

		void applyBgmVolume(int handle, float normalizedVolume) const;

		AudioRepository m_repository{};

		int       m_currentBgmHandle{ -1 };
		float     m_currentBgmVolume{ 0.0f };
		float     m_targetBgmVolume { 0.0f };
		FadeState m_fadeState       { FadeState::None };

		/** @brief フェードアウト完了後に再生する予約 BGM（-1 なら予約なし） */
		int   m_pendingBgmHandle{ -1 };
		float m_pendingBgmVolume{ 0.0f };
	};
} // namespace infrastructure
