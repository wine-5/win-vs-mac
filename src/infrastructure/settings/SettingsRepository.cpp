#include "SettingsRepository.h"
#include "core/utility/Log.h"
#include "thirdparty/nlohmann/json.hpp"
#include <algorithm>
#include <exception>
#include <fstream>

namespace
{
	/**
	 * @brief JSON から整数を読み、範囲内に収めて返す
	 *
	 * settings.json は人が開ける場所に平文で置くので、手で書き換えられている前提で扱う。
	 * 範囲外の値をそのまま通すと、音量が振り切れたり感度が0になって操作不能になる
	 * @param json 読み込み元のオブジェクト
	 * @param key 読み込むキー
	 * @param fallback キーが無い・型が違うときに使う値
	 * @param min 下限
	 * @param max 上限
	 * @return 範囲内に収めた値
	 */
	int readClampedInt(const nlohmann::json& json, const char* key, int fallback, int min, int max)
	{
		if (!json.contains(key) || !json[key].is_number_integer())
			return fallback;

		return std::clamp(json[key].get<int>(), min, max);
	}

	/**
	 * @brief JSON から真偽値を読む
	 * @param json 読み込み元のオブジェクト
	 * @param key 読み込むキー
	 * @param fallback キーが無い・型が違うときに使う値
	 * @return 読み込んだ値
	 */
	bool readBool(const nlohmann::json& json, const char* key, bool fallback)
	{
		if (!json.contains(key) || !json[key].is_boolean())
			return fallback;

		return json[key].get<bool>();
	}
} // namespace

namespace infrastructure::settings
{
	core::data::GameSettings SettingsRepository::load() const
	{
		core::data::GameSettings settings{};

		std::ifstream file{ FILE_PATH };
		if (!file.is_open())
			return settings; // 初回起動。既定値のまま始める

		try
		{
			nlohmann::json json{};
			file >> json;

			if (json.contains("audio"))
			{
				const auto& audio{ json["audio"] };
				constexpr int MAX_LEVEL{ core::data::AudioSettings::MAX_LEVEL };
				settings.m_audio.m_master = readClampedInt(audio, "master", settings.m_audio.m_master, 0, MAX_LEVEL);
				settings.m_audio.m_bgm = readClampedInt(audio, "bgm", settings.m_audio.m_bgm, 0, MAX_LEVEL);
				settings.m_audio.m_se = readClampedInt(audio, "se", settings.m_audio.m_se, 0, MAX_LEVEL);
			}

			if (json.contains("control"))
			{
				const auto& control{ json["control"] };
				using Control = core::data::ControlSettings;
				settings.m_control.m_sensitivity = readClampedInt(control, "sensitivity",
				    settings.m_control.m_sensitivity, Control::MIN_SENSITIVITY, Control::MAX_SENSITIVITY);
				settings.m_control.m_invertY = readBool(control, "invertY", settings.m_control.m_invertY);
				settings.m_control.m_screenShake = readClampedInt(control, "screenShake",
				    settings.m_control.m_screenShake, 0, Control::MAX_SHAKE);
			}
		}
		catch (const std::exception& e)
		{
			// 壊れたファイルは無視して既定値で続ける。ここで止めると、
			// 設定ファイルが1つ壊れただけでゲームが起動しなくなる
			core::log::warn("SettingsRepository::load: 設定を読めませんでした（既定値で続行）: {}", e.what());
			return core::data::GameSettings{};
		}

		return settings;
	}

	void SettingsRepository::save(const core::data::GameSettings& settings) const
	{
		const nlohmann::json json{
			{ "audio",
			    {
			        { "master", settings.m_audio.m_master },
			        { "bgm", settings.m_audio.m_bgm },
			        { "se", settings.m_audio.m_se },
			    } },
			{ "control",
			    {
			        { "sensitivity", settings.m_control.m_sensitivity },
			        { "invertY", settings.m_control.m_invertY },
			        { "screenShake", settings.m_control.m_screenShake },
			    } },
		};
		try
		{
			std::ofstream file{ FILE_PATH };
			if (!file.is_open())
			{
				core::log::warn("SettingsRepository::save: 設定を保存できませんでした（書き込み不可）");
				return;
			}

			// 人が開いて読める・直せる形で書く。設定ファイルは中身を見られる前提のもの
			file << json.dump(4) << '\n';
		}
		catch (const std::exception& e)
		{
			core::log::warn("SettingsRepository::save: 設定の保存に失敗しました: {}", e.what());
		}
	}
} // namespace infrastructure::settings
