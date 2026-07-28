#pragma once
#include <string_view>

namespace game::scene
{
    /**
     * @brief シーンの種類を定義
     */
    enum class SceneType
    {
        Bios,       // タイトル前のBIOS画面
        Lockscreen, // ロック画面
        Title,      // タイトル画面
        Select,     // セレクト画面
        Loading,    // ローディング画面
        InGame,     // ゲーム画面
		Result,     // リザルト画面

		// DEBUG: 破壊演出の検証用。方式が決まったら本体へ取り込んで削除する
		DebugDestruction
	};

	/**
	 * @brief シーン種別をログ表示用の名前へ変換する
	 * @param sceneType シーンの種類
	 * @return シーン名
	 */
	[[nodiscard]] constexpr std::string_view toString(SceneType sceneType) noexcept
	{
		switch (sceneType)
		{
		case SceneType::Bios: return "Bios";
		case SceneType::Lockscreen: return "Lockscreen";
		case SceneType::Title: return "Title";
		case SceneType::Select: return "Select";
		case SceneType::Loading: return "Loading";
		case SceneType::InGame: return "InGame";
		case SceneType::Result: return "Result";
		case SceneType::DebugDestruction: return "DebugDestruction";
		default: return "Unknown";
		}
	}
} // namespace game::scene