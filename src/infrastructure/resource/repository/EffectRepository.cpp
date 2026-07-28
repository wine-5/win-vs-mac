#include "EffectRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/effekseer/EffekseerForDXLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure::resource::repository
{
	EffectRepository::~EffectRepository()
	{
		for (const auto& [type, config] : m_configs)
		{
			if (config.m_handle != -1)
				DeleteEffekseerEffect(config.m_handle);
		}
	}

	void EffectRepository::initialize()
	{
		std::ifstream file{ "assets/data/effectData.json" };
		if (!file.is_open())
			throw std::runtime_error{ "assets/data/effectData.jsonを開けませんでした" };

		nlohmann::json json{};
		file >> json;

		load(json);
	}

	const std::unordered_map<core::constant::EffectType, EffectConfig>& EffectRepository::getAllConfigs() const
	{
		return m_configs;
	}

	void EffectRepository::load(const nlohmann::json& json)
	{
		if (!json.contains("effects")) return;

		for (const auto& entry : json["effects"])
		{
			const std::string key  { entry["type"] };
			const std::string path { entry["path"] };

			// 名前の綴り間違いや列挙への追加漏れを黙って捨てると、
			// 「エフェクトだけ出ない」状態の原因を追えなくなるため必ず気付けるようにする
			const core::constant::EffectType type{ core::constant::toEffectType(key) };
			if (type == core::constant::EffectType::None)
				throw std::runtime_error{ "effect '" + key + "' は EffectType に存在しません（EFFECT_TYPE_NAMESを確認）" };

			// efkファイルの配置漏れは配布物を作ったときに起きやすい。
			// ここで落としておかないと、遊ぶ側には「演出が無いゲーム」としか見えない
			int handle{ LoadEffekseerEffect(path.c_str()) };
			if (handle == -1)
				throw std::runtime_error{ "エフェクトファイル '" + path + "' を読み込めませんでした" };

			if (!entry.contains("poolSize") || !entry.contains("yOffset") || !entry.contains("scale"))
				throw std::runtime_error{ "effect '" + key + "' に必須フィールド (poolSize / yOffset / scale) が設定されていません" };

			EffectConfig config{};
			config.m_handle   = handle;
			config.m_poolSize = entry["poolSize"].get<int>();
			config.m_yOffset  = entry["yOffset"].get<float>();
			config.m_scale    = entry["scale"].get<float>();

			m_configs[type] = config;
		}
	}
} // namespace infrastructure::resource::repository