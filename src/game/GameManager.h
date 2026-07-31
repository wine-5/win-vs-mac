#pragma once
#include "game/data/FileEquipmentData.h"
#include "core/base/NonCopyable.h"
#include "core/data/Difficulty.h"
#include "core/constant/DebugFlags.h"
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
		 * @brief 連続ジャンプが有効かどうかを返す
		 * @return 有効ならtrue（PhysicsSystemが接地・押下エッジ判定を無効化する）
		 */
		[[nodiscard]] bool isContinuousJumpEnabled() const noexcept
		{
			return m_continuousJumpEnabled;
		}

		// DEBUG: ここまでデバッグモード関連

		/**
		 * @brief セレクト画面のチュートリアルを出すべきかを返し、以後は出ないようにする
		 *
		 * 初見の「何をすればいいのか分からない」を解くための案内なので、
		 * 一度見せたら以降の再挑戦では出さない。アプリを起動し直せばまた出る
		 * （面接や展示では毎回起動し直すため、保存せずともかならず見せられる）
		 * @return 今回出すべきならtrue
		 */
		[[nodiscard]] bool consumeSelectTutorial() noexcept
		{
			const bool shouldShow{ m_isSelectTutorialPending };
			m_isSelectTutorialPending = false;
			return shouldShow;
		}

		/**
		 * @brief デフォルトコンストラクタ
		 */
		GameManager() = default;

    private:

        data::FileEquipmentData m_fileEquipmentData{};
        core::data::ResultData m_resultData{};
		core::data::Difficulty m_difficulty{ core::data::Difficulty::Normal };
		bool m_quitRequested{ false };

		// セレクト画面のチュートリアルをまだ見せていないか（起動直後の1回だけ出す）
		bool m_isSelectTutorialPending{ true };

		// DEBUG: 連続ジャンプ（空中浮上）を許可するか。切り替えは DebugFlags.h で行う
		bool m_continuousJumpEnabled{ core::constant::ALLOW_CONTINUOUS_JUMP };
	};
} // namespace game