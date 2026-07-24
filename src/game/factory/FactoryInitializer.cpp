#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/utility/MathConstants.h"
#include "core/data/PropDefinition.h"
#include "game/constant/ModelId.h"
#include "game/data/GroundData.h"
#include <stdexcept>
#include <cmath>

namespace game::factory
{
	FactoryInitializer::FactoryInitializer(
		FactoryManager& factoryManager,
		core::iface::IResourceManager& resourceManager)
		: m_factoryManager{factoryManager}
		, m_resourceManager{resourceManager}
	{
	}

	void FactoryInitializer::initializePlayer(const data::PlayerData& playerData)
	{
		int playerHandle{m_resourceManager.loadModelById(constant::model_id::PLAYER)};
		m_factoryManager.getPlayerFactory().create(playerHandle, playerData);
	}

	core::ecs::EntityId FactoryInitializer::initializeGround()
	{
		int groundHandle{m_resourceManager.loadModelById(constant::model_id::GROUND)};
		auto groundMeta{m_resourceManager.getMetadata(constant::model_id::GROUND)};
		if (!groundMeta.has_value()) {
			core::log::info("ERROR: Groundのメタデータが見つかりません");
			throw std::runtime_error("Groundのメタデータの読み込みに失敗しました");
		}

		data::GroundData groundData = data::GroundData::fromMetadata(groundMeta.value());
		return m_factoryManager.getGroundFactory().create(groundHandle, groundData);
	}

	void FactoryInitializer::initializeProps()
	{
		const auto& stage{ m_resourceManager.getStageMetadata() };
		auto& factory{ m_factoryManager.getStagePropFactory() };

		for (const auto& prop : stage.m_props)
		{
			const auto& def{ m_resourceManager.getPropDefinition(prop.m_type) };
			const int handle{ m_resourceManager.loadModelByPath(def.m_modelPath) };

			// 実寸(size) ÷ 素材実寸(baseSize) をモデルスケールにする。
			// baseSizeが0の軸は割れないためスケール1にフォールバックする
			const core::Vector3 scale{
				def.m_baseSize.x != 0.0f ? prop.m_size.x / def.m_baseSize.x : 1.0f,
				def.m_baseSize.y != 0.0f ? prop.m_size.y / def.m_baseSize.y : 1.0f,
				def.m_baseSize.z != 0.0f ? prop.m_size.z / def.m_baseSize.z : 1.0f
			};

			// JSONは度数法で持つ。DxLibのMV1SetRotationXYZはラジアンなので変換する
			const core::Vector3 rotation{ prop.m_rotation * core::utility::DEG_TO_RAD };

			// カタログで "box" 指定のものだけコライダーを付ける（size全0でStageProp側が判定を省く）。
			// コライダーは軸並行(AABB)なので、Y回転を反映して footprint を実物に合わせる
			// （壁を90°横に置くと幅と奥行きが入れ替わる）。X/Z回転（坂）はAABBで正確に
			// 表せないため考慮しない（坂の接地はレイキャストで別途対応する）
			core::Vector3 colliderSize{};
			if (def.m_collider == "box")
			{
				const float yaw{ rotation.y };
				const float cosYaw{ std::abs(std::cos(yaw)) };
				const float sinYaw{ std::abs(std::sin(yaw)) };
				colliderSize = core::Vector3{
					prop.m_size.x * cosYaw + prop.m_size.z * sinYaw,
					prop.m_size.y,
					prop.m_size.x * sinYaw + prop.m_size.z * cosYaw
				};
			}

			factory.create(handle, prop.m_position, rotation, scale, colliderSize);
		}
	}
} // namespace game::factory
