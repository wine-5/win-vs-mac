#pragma once

namespace platform::window
{
    /**
     * @class WindowConstants
     * @brief ウィンドウ通信で使用される共有定数クラス
     */
    class WindowConstants
    {
    public:
        // JSONキー（複数ウィンドウで共有）
        static constexpr const char* JSON_KEY_TYPE{ "type" };
        static constexpr const char* JSON_KEY_WINDOW{ "window" };
        static constexpr const char* JSON_KEY_APP{ "app" };
        static constexpr const char* JSON_KEY_VISIBLE{ "visible" };
        static constexpr const char* JSON_KEY_JOB{ "job" };
        static constexpr const char* JSON_KEY_STATS{ "stats" };
        static constexpr const char* JSON_KEY_ID{ "id" };
        static constexpr const char* JSON_KEY_HP{ "hp" };
        static constexpr const char* JSON_KEY_ATK{ "atk" };
        static constexpr const char* JSON_KEY_DEF{ "def" };
        static constexpr const char* JSON_KEY_SPD{ "spd" };
        static constexpr const char* JSON_KEY_SLOT{ "slot" };

        // JSONキー（ParameterWindow）
        static constexpr const char* JSON_KEY_SKILL{ "skill" };
        static constexpr const char* JSON_KEY_BASE_HP{ "baseHp" };
        static constexpr const char* JSON_KEY_BASE_ATK{ "baseAtk" };
        static constexpr const char* JSON_KEY_BASE_DEF{ "baseDef" };
        static constexpr const char* JSON_KEY_BASE_SPD{ "baseSpd" };
        static constexpr const char* JSON_KEY_BONUS_HP{ "bonusHp" };
        static constexpr const char* JSON_KEY_BONUS_ATK{ "bonusAtk" };
        static constexpr const char* JSON_KEY_BONUS_DEF{ "bonusDef" };
        static constexpr const char* JSON_KEY_BONUS_SPD{ "bonusSpd" };

        // JSONキー（FileSelectWindow）
        static constexpr const char* JSON_KEY_FILE_SLOT{ "slot" };
        static constexpr const char* JSON_KEY_IS_EMPTY{ "isEmpty" };
        static constexpr const char* JSON_KEY_FILE_NAME{ "fileName" };
        static constexpr const char* JSON_KEY_FILE_PATH{ "filePath" };
        static constexpr const char* JSON_KEY_EXT_TYPE{ "extType" };
        static constexpr const char* JSON_KEY_DESCRIPTIONS{ "descs" };

        // JSONメッセージタイプ（Win32SelectWindowManager）
        static constexpr const char* MESSAGE_TYPE_START_GAME{ "startGame" };
        static constexpr const char* MESSAGE_TYPE_TOGGLE_WINDOW{ "toggleWindow" };
        static constexpr const char* MESSAGE_TYPE_LAUNCH_APP{ "launchApp" };
        static constexpr const char* MESSAGE_TYPE_WINDOW_STATE_CHANGED{ "windowStateChanged" };

        // JSONメッセージタイプ（JobWindow）
        static constexpr const char* MESSAGE_TYPE_JOB_SELECTED{ "jobSelected" };
        static constexpr const char* MESSAGE_TYPE_REQUEST_JOB_STATS{ "requestJobStats" };
        static constexpr const char* MESSAGE_TYPE_JOB_STATS{ "jobStats" };

        // JSONメッセージタイプ（FileSelectWindow）
        static constexpr const char* MESSAGE_TYPE_SLOT_SELECTED{ "slotSelected" };
        static constexpr const char* MESSAGE_TYPE_REQUEST_BONUS_INFO{ "requestBonusInfo" };
        static constexpr const char* MESSAGE_TYPE_REFRESH{ "refresh" };
        static constexpr const char* MESSAGE_TYPE_BONUS_INFO{ "bonusInfo" };

        // JSONメッセージタイプ（ResultWindow）
        static constexpr const char* MESSAGE_TYPE_REQUEST_RESULT{ "requestResult" };
        static constexpr const char* MESSAGE_TYPE_RESULT_DATA{ "resultData" };
        static constexpr const char* MESSAGE_TYPE_RETRY{ "retry" };
        static constexpr const char* MESSAGE_TYPE_TITLE{ "title" };

        // JSONメッセージタイプ（LoadingWindow）
        static constexpr const char* MESSAGE_TYPE_LOADING_COMPLETE{ "loadingComplete" };

    private:
        WindowConstants() = delete;
    };
}
