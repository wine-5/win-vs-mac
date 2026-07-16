#include "ProjectileRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"

namespace
{
	/**
	 * @brief JSONのprojectile要素をProjectileMetadataへ変換する
	 * @param j projectile要素のJSONオブジェクト
	 * @return 変換したProjectileMetadata
	 */
	core::data::ProjectileMetadata parseProjectile(const nlohmann::json& j)
	{
		core::data::ProjectileMetadata metadata{};
		metadata.m_id = j["id"].get<std::string>();
		metadata.m_speed = j["speed"];
		metadata.m_damage = j["damage"];
		metadata.m_lifetime = j["lifetime"];
		metadata.m_radius = j["radius"];
		metadata.m_scale = j["scale"];
		metadata.m_spawnForward = j["spawnForward"];
		metadata.m_spawnHeight = j["spawnHeight"];
		metadata.m_cooldown = j["cooldown"];
		return metadata;
	}
} // namespace

namespace infrastructure
{
	ProjectileRepository::ProjectileRepository()
	{
		std::ifstream file("assets/data/projectileData.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/projectileData.jsonを開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);
		for (const auto& projectile : j["projectiles"])
		{
			core::data::ProjectileMetadata metadata{ parseProjectile(projectile) };
			m_metadata[metadata.m_id] = metadata;
		}
	}

	const core::data::ProjectileMetadata& ProjectileRepository::getMetadata(std::string_view projectileId) const
	{
		auto it{ m_metadata.find(std::string(projectileId)) };
		if (it == m_metadata.end())
			throw std::runtime_error("弾ID '" + std::string(projectileId) + "' がprojectileData.jsonに存在しません");
		return it->second;
	}
} // namespace infrastructure
