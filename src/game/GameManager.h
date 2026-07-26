#pragma once
#include "game/data/FileEquipmentData.h"
#include "core/base/NonCopyable.h"
#include "core/data/Difficulty.h"
#include "core/data/ResultData.h"

namespace game
{
	/**
	 * @brief ゲーム全体の状態を管理するクラス
	 * Application が唯一のインスタンスを所有し、シーン間で共有するデータを保持する。
	 * 利用側へはコンストラクタで参照注入する（Singletonによる暗黙の横断参照を避ける）。
	 */
	class GameManager : private core::base::NonCopyable
	{
    public:
        /**
         * @brief FileEquipmentData への参照を返す
         * @return FileEquipmentData の参照
         */
        [[nodiscard]] data::FileEquipmentData& getFileEquipmentData() noexcept
        {
            return m_fileEquipmentData;
        }

		/**
		 * @brief 難易度を設定する
		 *
		 * セレクト画面の難易度ウィンドウで選択が変わるたびに呼ばれる。
		 * @param difficulty 選択された難易度
		 */
		void setDifficulty(core::data::Difficulty difficulty) noexcept
		{
			m_difficulty = difficulty;
		}

		/**
		 * @brief 現在の難易度を取得する
		 * @return 選択されている難易度（未選択なら Normal）
		 */
		[[nodiscard]] core::data::Difficulty getDifficulty() const noexcept
		{
			return m_difficulty;
		}

		/**
		 * @brief アプリケーションの終了を要求する
		 *
		 * 実際の終了はApplicationがメインループの条件で検知して行う。
		 * デストラクタとDxLib_Endを正しく通すため、std::exitは使わない。
		 */
		void requestQuit() noexcept
		{
			m_quitRequested = true;
		}

		/**
		 * @brief 終了が要求されているかを返す
		 * @return 要求されている場合true
		 */
		[[nodiscard]] bool isQuitRequested() const noexcept
		{
			return m_quitRequested;
		}

		/**
         * @brief ResultData を保存する
         * @param data リザルトデータ
         */
        void setResultData(const core::data::ResultData& data) noexcept
        {
            m_resultData = data;
        }

        /**
         * @brief ResultData への参照を返す
         * @return ResultData の定数参照
         */
        [[nodiscard]] const core::data::ResultData& getResultData() const noexcept
        {
            return m_resultData;
        }

		// DEBUG: ここからデバッグモード関連（リリース時にまとめて削除する）

		/**
		 * @brief デバッグモードのON/OFFを切り替える
		 */
		void toggleDebugMode() noexcept
		{
			m_debugMode = !m_debugMode;
		}

		/**
		 * @brief デバッグモードが有効かどうかを返す
		 * @return 有効ならtrue
		 */
		[[nodiscard]] bool isDebugMode() const noexcept
		{
			return m_debugMode;
		}

		/**
		 * @brief 連続ジャンプ（空中でも押し続けで浮上）のON/OFFを切り替える
		 *
		 * 通常は接地時の単発ジャンプに制限するが、デバッグで空中移動したいとき用に残す。
		 */
		void toggleContinuousJump() noexcept
		{
			m_continuousJumpEnabled = !m_continuousJumpEnabled;
		}

		/**
		 * @brief 連続ジャンプが有効かどうかを返す
		 * @return 有効ならtrue（PhysicsSystemが接地・押下エッジ判定を無効化する）
		 */
		[[nodiscard]] bool isContinuousJumpEnabled() const noexcept
		{
			return m_continuousJumpEnabled;
		}

		// DEBUG: ここまでデバッグモード関連

		/**
		 * @brief デフォルトコンストラクタ
		 */
		GameManager() = default;

    private:

        data::FileEquipmentData m_fileEquipmentData{};
        core::data::ResultData m_resultData{};
		core::data::Difficulty m_difficulty{ core::data::Difficulty::Normal };
		bool m_quitRequested{ false };

		// DEBUG: デバッグモードの状態（リリース時に削除）
		bool m_debugMode{ false };
		// DEBUG: 連続ジャンプ（空中浮上）を許可するか。falseで通常の接地単発ジャンプ（リリース時に削除）
		bool m_continuousJumpEnabled{ true };
	};
} // namespace game