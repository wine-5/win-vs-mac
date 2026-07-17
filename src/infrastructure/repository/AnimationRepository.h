#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

namespace infrastructure::repository
{
	/**
	 * @brief アニメーションリソースを管理するリポジトリクラス
	 *
	 * resources.json の "animations" セクションからパスを読み込み、
	 * ID でアニメーションモデルハンドルを取得する
	 */
	class AnimationRepository
	{
	public:
		/**
		 * @brief コンストラクタ
		 *
		 * resources.json の "animations" セクションを読み込む
		 * @throw std::runtime_error ファイルが見つからないか JSON パースに失敗した場合
		 */
		AnimationRepository();

		/// @brief デストラクタ（ロード済みハンドルを解放）
		~AnimationRepository();

		/**
		 * @brief ID でアニメーションモデルを読み込みハンドルを返す（キャッシュ付き）
		 * @param animationId アニメーション ID
		 * @return DxLib モデルハンドル、失敗時は -1
		 */
		int loadAnimationById(std::string_view animationId);

	private:
		std::unordered_map<std::string, std::string> m_paths{};
		std::unordered_map<std::string, int>         m_handles{};
	};
} // namespace infrastructure::repository
