#pragma once
#include "core/base/NonCopyable.h"
#include "core/data/GameSettings.h"

namespace core::iface
{
	class ISettingsRepository; // 前方宣言
}

namespace game
{
	/**
	 * @brief プレイヤーが変更した設定を保持し、反映と保存を行うクラス
	 *
	 * Application が唯一のインスタンスを所有し、シーン間をまたいで生存する。
	 * 設定画面（タイトル・ポーズ・セレクト）はどこから開いても同じここを書き換えるため、
	 * 画面ごとに値がずれることがない。
	 * 利用側へはコンストラクタで参照注入する（PauseManager と同じ扱い）
	 */
	class SettingsManager : private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief SettingsManagerのコンストラクタ（保存済みの設定を読み込む）
		 * @param repository 設定の保存・読み込みを行うリポジトリ
		 */
		explicit SettingsManager(core::iface::ISettingsRepository& repository);

		/**
		 * @brief 現在の音量設定を返す
		 * @return 音量設定
		 */
		[[nodiscard]] const core::data::AudioSettings& getAudio() const noexcept
		{
			return m_settings.m_audio;
		}

		/**
		 * @brief 現在の操作設定を返す
		 * @return 操作設定
		 */
		[[nodiscard]] const core::data::ControlSettings& getControl() const noexcept
		{
			return m_settings.m_control;
		}

		/**
		 * @brief 音量設定を差し替え、その場で音へ反映する
		 * @param audio 新しい音量設定
		 */
		void setAudio(const core::data::AudioSettings& audio);

		/**
		 * @brief 操作設定を差し替える
		 *
		 * 参照している側（CameraSystem など）が毎フレーム読みに来るため、
		 * ここでは値を書き換えるだけでよい
		 * @param control 新しい操作設定
		 */
		void setControl(const core::data::ControlSettings& control);

		/**
		 * @brief いまの音量設定を AudioManager へ反映する
		 *
		 * AudioManager はサービス初期化の途中で生まれるため、起動直後に一度呼んで
		 * 保存されていた音量を行き渡らせる必要がある
		 */
		void applyAudio() const;

		/**
		 * @brief 変更があれば保存する
		 *
		 * スライダーを動かすたびに書くとドラッグ中に毎フレーム書き込むことになるため、
		 * 設定画面を閉じるときにまとめて呼ぶ
		 */
		void save();

	  private:
		core::iface::ISettingsRepository& m_repository;

		core::data::GameSettings m_settings{};

		/** @brief 前回の保存から変更があったか（無駄な書き込みを避ける） */
		bool m_isDirty{ false };
	};
} // namespace game
