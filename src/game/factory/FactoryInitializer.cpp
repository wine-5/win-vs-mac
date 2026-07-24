#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/utility/MathConstants.h"
#include "core/data/PropDefinition.h"
#include "game/constant/ModelId.h"
#include "game/constant/PropCollision.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/visual/LightComponent.h"
#include <cmath>
#include <algorithm>

namespace game::factory
{
	FactoryInitializer::FactoryInitializer(
	    FactoryManager& factoryManager,
	    core::iface::IResourceManager& resourceManager,
	    core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager)
	    : m_factoryManager{ factoryManager }
	    , m_resourceManager{ resourceManager }
	    , m_entityManager{ entityManager }
	    , m_componentManager{ componentManager }
	{
	}

	void FactoryInitializer::initializeLights()
	{
		const auto& stage{ m_resourceManager.getStageMetadata() };

		for (const auto& lightData : stage.m_lights)
		{
			const auto entity{ m_entityManager.create() };

			component::movement::TransformComponent transform{};
			transform.m_position = lightData.m_position;
			m_componentManager.add<component::movement::TransformComponent>(entity.getId(), transform);

			// 位置はTransformが持つので、Component側のoffsetは0のままでよい
			component::visual::LightComponent light{};
			light.m_range = lightData.m_range;
			light.m_r = lightData.m_r;
			light.m_g = lightData.m_g;
			light.m_b = lightData.m_b;
			m_componentManager.add<component::visual::LightComponent>(entity.getId(), light);
		}
	}

	void FactoryInitializer::initializePlayer(const data::PlayerData& playerData)
	{
		int playerHandle{m_resourceManager.loadModelById(constant::model_id::PLAYER)};
		m_factoryManager.getPlayerFactory().create(playerHandle, playerData);
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

			stage::StagePropParams params{};
			params.m_modelHandle = handle;
			params.m_position = prop.m_position;
			params.m_rotation = rotation;
			params.m_scale = scale;

			const auto collision{ constant::toPropCollision(def.m_collider) };
			params.m_collision = collision;

			// Box（壁・柱）は軸並行(AABB)で押し返すため、Y回転を反映して footprint を
			// 実物に合わせる（壁を90°横に置くと幅と奥行きが入れ替わる）。
			// Ground（床・坂）は傾きごと GroundingSystem が扱うので実寸をそのまま渡す
			if (collision == constant::PropCollision::Box)
			{
				const float cosYaw{ std::abs(std::cos(rotation.y)) };
				const float sinYaw{ std::abs(std::sin(rotation.y)) };
				params.m_collisionSize = core::Vector3{
					prop.m_size.x * cosYaw + prop.m_size.z * sinYaw,
					prop.m_size.y,
					prop.m_size.x * sinYaw + prop.m_size.z * cosYaw
				};
			}
			else if (collision == constant::PropCollision::Ground)
			{
				params.m_collisionSize = prop.m_size;
				params.m_slideAccel = def.m_slideAccel;
			}

			// テクスチャ1枚が受け持つ実寸から繰り返し回数を決める。
			// U/Vは面の向きに合わせる。床（Yが最も薄い）は上面を見るのでX×Z、
			// 壁や柱は側面を見るので「横幅×高さ」を割り当てる。
			// 1未満にすると繰り返しではなく絵の一部を引き伸ばす（切り取る）ため下限を1にする
			if (def.m_textureTile > 0.0f)
			{
				const bool isFloorLike{ prop.m_size.y <= prop.m_size.x && prop.m_size.y <= prop.m_size.z };
				const float horizontal{ isFloorLike ? prop.m_size.x : std::max(prop.m_size.x, prop.m_size.z) };
				const float vertical{ isFloorLike ? prop.m_size.z : prop.m_size.y };

				params.m_uvScaleU = std::max(1.0f, horizontal / def.m_textureTile);
				params.m_uvScaleV = std::max(1.0f, vertical / def.m_textureTile);
			}

			factory.create(params);
		}
	}
} // namespace game::factory
