#include "AudioRepository.h"
#include <fstream>
#include <stdexcept>
#include "DxLib.h"

namespace infrastructure::repository
{
	AudioRepository::~AudioRepository()
	{
		for (const auto& [type, config] : m_bgmConfigs)
		{
			if (config.m_handle != -1)
				DeleteSoundMem(config.m_handle);
		}
		for (const auto& [type, config] : m_seConfigs)
		{
			if (config.m_handle != -1)
				DeleteSoundMem(config.m_handle);
		}
	}

	void AudioRepository::initialize()
	{
		std::ifstream file{ "assets/config/resources.json" };
		if (!file.is_open())
			throw std::runtime_error{ "resources.json を開けませんでした" };

		nlohmann::json json{};
		file >> json;

		loadBgm(json);
		loadSe(json);
	}

	const std::unordered_map<core::constant::BgmType, BgmConfig>& AudioRepository::getAllBgmConfigs() const
	{
		return m_bgmConfigs;
	}

	const std::unordered_map<core::constant::SeType, SeConfig>& AudioRepository::getAllSeConfigs() const
	{
		return m_seConfigs;
	}

	void AudioRepository::loadBgm(const nlohmann::json& json)
	{
		if (!json.contains("bgm")) return;

		const std::unordered_map<std::string, core::constant::BgmType> typeMap
		{
			{ "Title",      core::constant::BgmType::Title      },
			{ "Select",     core::constant::BgmType::Select     },
			{ "InGame",     core::constant::BgmType::InGame     },
			{ "Boss",       core::constant::BgmType::Boss       },
			{ "ResultWin",  core::constant::BgmType::ResultWin  },
			{ "ResultLose", core::constant::BgmType::ResultLose },
		};

		for (const auto& entry : json["bgm"])
		{
			const std::string key  { entry["type"] };
			const std::string path { entry["path"] };

			auto it{ typeMap.find(key) };
			if (it == typeMap.end()) continue;

			if (!entry.contains("volume"))
				throw std::runtime_error{ "bgm '" + key + "' に必須フィールド (volume) が設定されていません" };

			int handle{ LoadSoundMem(path.c_str()) };
			if (handle == -1) continue;

			BgmConfig config{};
			config.m_handle  = handle;
			config.m_volume  = entry["volume"].get<float>();
			config.m_useFade = entry.value("useFade", true);

			m_bgmConfigs[it->second] = config;
		}
	}

	void AudioRepository::loadSe(const nlohmann::json& json)
	{
		if (!json.contains("se")) return;

		const std::unordered_map<std::string, core::constant::SeType> typeMap
		{
			{ "AttackWarrior",   core::constant::SeType::AttackWarrior   },
			{ "AttackFire",      core::constant::SeType::AttackFire      },
			{ "AttackNinja",     core::constant::SeType::AttackNinja     },
			{ "HitEnemy",        core::constant::SeType::HitEnemy        },
			{ "HitPlayer",       core::constant::SeType::HitPlayer       },
			{ "DeadEnemy",       core::constant::SeType::DeadEnemy       },
		};

		for (const auto& entry : json["se"])
		{
			const std::string key  { entry["type"] };
			const std::string path { entry["path"] };

			auto it{ typeMap.find(key) };
			if (it == typeMap.end()) continue;

			if (!entry.contains("volume"))
				throw std::runtime_error{ "se '" + key + "' に必須フィールド (volume) が設定されていません" };

			int handle{ LoadSoundMem(path.c_str()) };
			if (handle == -1) continue;

			SeConfig config{};
			config.m_handle = handle;
			config.m_volume = entry["volume"].get<float>();

			m_seConfigs[it->second] = config;
		}
	}
} // namespace infrastructure::repository
