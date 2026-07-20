#pragma once
#include "game/data/FileEquipmentData.h"
#include "game/data/JobSelectionData.h"
#include "core/base/Singleton.h"
#include "core/data/ResultData.h"

namespace game
{
    /**
     * @brief ゲーム全体の状態を管理するクラス
     * Singleton として唯一のインスタンスが存在し、シーン間で共有するデータを保持する
     */
    class GameManager : public core::base::Singleton<GameManager>
    {
    public:
        friend core::base::Singleton<GameManager>;

        /**
         * @brief FileEquipmentData への参照を返す
         * @return FileEquipmentData の参照
         */
        [[nodiscard]] data::FileEquipmentData& getFileEquipmentData() noexcept
        {
            return m_fileEquipmentData;
        }

        /**
         * @brief JobSelectionData への参照を返す
         * @return JobSelectionData の参照
         */
        [[nodiscard]] data::JobSelectionData& getJobSelectionData() noexcept
        {
            return m_jobSelectionData;
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

		// DEBUG: ここまでデバッグモード関連

		/**
         * @brief デフォルトコンストラクタ（Singleton初期化用）
         */
        GameManager() = default;

    private:

        data::FileEquipmentData m_fileEquipmentData{};
        data::JobSelectionData m_jobSelectionData{};
        core::data::ResultData m_resultData{};

		// DEBUG: デバッグモードの状態（リリース時に削除）
		bool m_debugMode{ false };
	};
} // namespace game