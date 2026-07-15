#pragma once
#include <string>
#include <unordered_map>
#include "core/constant/BgmType.h"
#include "core/constant/SeType.h"
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure
{
	/**
	 * @brief BGM リソースの設定情報
	 */
	struct BgmConfig
	{
		int   m_handle{ -1 };
		float m_volume{ 1.0f };
		bool  m_useFade{ true };
	};

	/**
	 * @brief SE リソースの設定情報
	 */
	struct SeConfig
	{
		int   m_handle{ -1 };
		float m_volume{ 1.0f };
	};
	/**
	 * @brief BGM・SE のサウンドリソースを管理するリポジトリクラス
	 *
	 * @details resources.json からサウンドファイルパスと設定を読み込み、
	 * LoadSoundMem() でハンドルを取得・キャッシュする
	 */
	class AudioRepository
	{
	public:
		AudioRepository() = default;
		~AudioRepository();

		/**
		 * @brief resources.json からサウンドリソースを読み込む
		 * @throw std::runtime_error ファイルが見つからないか、JSON パースに失敗した場合
		 */
		void initialize();

		/**
		* @brief 全 BGM 設定を返す
		* @return BgmType → BgmConfig のマップ
		*/
		[[nodiscard]] const std::unordered_map<core::constant::BgmType, BgmConfig>& getAllBgmConfigs() const;

		/**
		 * @brief 全 SE 設定を返す
		 * @return SeType → SeConfig のマップ
		 */
		[[nodiscard]] const std::unordered_map<core::constant::SeType, SeConfig>& getAllSeConfigs() const;

	private:
		void loadBgm(const nlohmann::json& json);
		void loadSe(const nlohmann::json& json);

		std::unordered_map<core::constant::BgmType, BgmConfig> m_bgmConfigs{};
		std::unordered_map<core::constant::SeType, SeConfig>  m_seConfigs{};
	};
} // namespace infrastructure