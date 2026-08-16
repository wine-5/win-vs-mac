#pragma once
#include "core/interface/IAudioManager.h"
#include "infrastructure/resource/repository/AudioRepository.h"

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
		 * @brief 現在再生中の BGM を停止する（デストラクタからの内部利用）
		 * @param fade true の場合フェードアウトを使用する（デフォルト: true）
		 */
		void stopBgm(bool fade = true);

		/**
		 * @brief SE を再生する
		 * @param type 再生する SE の種別
		 */
		void playSe(core::constant::SeType type) override;

		/**
		 * @brief フェード処理など毎フレームの更新処理
		 */
		void update(float deltaTime) override;

		/**
		 * @brief プレイヤーが設定した音量を反映する
		 * @param settings 反映する音量設定
		 */
		void applyVolumeSettings(const core::data::AudioSettings& settings) override;

	  private:
		/** @brief フェード処理の種類 */
		enum class FadeState
		{
			None,
			FadeIn,
			FadeOut,
		};

		/**
		 * @brief フェードにかける時間（秒）
		 *
		 * 従来は「1フレームあたりの変化量0.01」で、60fps換算だと約1.67秒だった。
		 * 他の演出（FadeTransition等）が秒指定なのに合わせ、フレームレート非依存にする
		 */
		static constexpr float FADE_DURATION{ 1.67f };

		/** @brief DxLib の音量指定の最大値 */
		static constexpr int DX_VOLUME_MAX{ 255 };

		/**
		 * @brief 再生中の BGM へ、いまの状態から求めた音量を掛け直す
		 *
		 * フェードの進み具合・音源ごとの音量・プレイヤーの設定を掛け合わせた結果を渡す。
		 * 掛け算の順序をここ1か所に閉じることで、設定を変えた瞬間とフェード中が衝突しない
		 */
		void applyBgmVolume() const;

		resource::repository::AudioRepository m_repository{};

		/** @brief プレイヤーが設定した音量（音源ごとの音量へ掛ける倍率のもと） */
		core::data::AudioSettings m_volumeSettings{};

		int m_currentBgmHandle{ -1 };

		/**
		 * @brief フェードの進み具合（0.0＝無音、1.0＝フェード完了）
		 *
		 * 実音量そのものではなく係数として持つ。実音量を直接補間すると、
		 * フェード中に音量設定を変えたときに両者が同じ変数を奪い合ってしまう
		 */
		float m_fadeLevel{ 0.0f };

		/** @brief 再生中の BGM の音源ごとの音量（resources.json の値） */
		float m_currentBgmSourceVolume{ 0.0f };

		FadeState m_fadeState{ FadeState::None };

		/** @brief フェードアウト完了後に再生する予約 BGM（-1 なら予約なし） */
		int m_pendingBgmHandle{ -1 };
		float m_pendingBgmSourceVolume{ 0.0f };
	};
} // namespace infrastructure
