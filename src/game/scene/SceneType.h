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
		Result      // リザルト画面
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
		default: return "Unknown";
		}
	}
} // namespace game::scene