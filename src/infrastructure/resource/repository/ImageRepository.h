#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure::resource::repository
{
    /**
     * @brief 画像リソースを管理するリポジトリクラス
     *
     * resources.json の "images" セクションからパスを読み込み、
     * ID でハンドルを取得する
     */
    class ImageRepository
    {
    public:
        /**
         * @brief コンストラクタ
         *
         * resources.json の "images" セクションを読み込む
         * @throw std::runtime_error ファイルが見つからないか JSON パースに失敗した場合
         */
	  ImageRepository(const nlohmann::json& j);

	  /// @brief デストラクタ（ロード済みハンドルを解放）
	  ~ImageRepository();

	  /**
	   * @brief ID で画像を読み込みハンドルを返す（キャッシュ付き）
	   * @param imageId 画像 ID
	   * @return DxLib 画像ハンドル、失敗時は -1
	   */
	  int loadImageById(std::string_view imageId);

    private:
        std::unordered_map<std::string, std::string> m_paths{};
        std::unordered_map<std::string, int>         m_handles{};
    };
} // namespace infrastructure::resource::repository
