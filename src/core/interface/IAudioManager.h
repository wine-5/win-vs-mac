#pragma once
#include "core/constant/BgmType.h"
#include "core/constant/SeType.h"

namespace core::iface
{
	/**
	 * @brief BGM・SE の再生・停止を行うインターフェース
	 * Game 層が Infrastructure 層に依存しないようにするためのインターフェース
	 */
	class IAudioManager
	{
	public:
		virtual ~IAudioManager() = default;

		/**
		 * @brief 起動時にサウンドリソースを読み込む
		 * @throw std::runtime_error ファイルが見つからないか、JSON パースに失敗した場合
		 */
		virtual void initialize() = 0;

		/**
		 * @brief BGM を再生する
		 * @param type 再生する BGM の種別
		 * @param fade true の場合フェードイン・アウトを使用する（デフォルト: true）
		 */
		virtual void playBgm(core::constant::BgmType type, bool fade = true) = 0;

		/**
		 * @brief SE を再生する
		 * @param type 再生する SE の種別
		 */
		virtual void playSe(core::constant::SeType type) = 0;

		/**
		 * @brief フェード処理など毎フレームの更新処理
		 * @param deltaTime フレーム間の時間差（秒）
		 */
		virtual void update(float deltaTime) = 0;
	};
} // namespace core::iface