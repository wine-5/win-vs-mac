#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <vector>
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure::repository
{
	/**
	 * @brief フォントリソースを管理するリポジトリクラス
	 *
	 * resources.jsonからフォント情報を読み込み、
	 * DxLibのAddFontResourceExでフォントを登録する
	 */
	class FontRepository
	{
	public:
		/**
		 * @brief コンストラクタ
		 *
		 * コンストラクト時にresources.jsonからすべてのフォントを読み込み、
		 * DxLibに登録する
		 * @throw std::runtime_error ファイルが見つからないか、JSONパースに失敗した場合
		 */
		FontRepository();

		/**
		 * @brief デストラクタ
		 *
		 * デストラクト時に登録済みフォントをRemoveFontResourceExでクリーンアップする
		 */
		~FontRepository();

		/**
		 * @brief フォントIDからフォント名を取得する
		 *
		 * @param fontId フォント識別子
		 * @return フォントファミリー名（存在しない場合nullopt）
		 */
		std::optional<std::string> getFontName(std::string_view fontId) const;

	private:
		struct FontDefinition
		{
			std::string m_id;
			std::string m_path;
			std::string m_name;
		};

		std::vector<FontDefinition> loadFontList(const nlohmann::json& json);

		std::unordered_map<std::string, std::string> m_fontNames;
		std::unordered_map<std::string, std::string> m_fontPaths;
	};
} // namespace infrastructure::repository