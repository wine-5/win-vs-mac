#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/utility/MathConstants.h"
#include "core/data/PropDefinition.h"
#include "game/constant/ModelId.h"
#include "game/constant/PropCollision.h"
#include "game/data/GroundData.h"
#include <stdexcept>
#include <cmath>
#include <utility>

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
			// 面の見え方を決めるのは大きい2辺なので、それをU/Vに割り当てる
			// （壁なら「長さ×高さ」、床なら「奥行×幅」が対象になる）
			if (def.m_textureTile > 0.0f)
			{
				float largest{ prop.m_size.x };
				float second{ prop.m_size.y };
				float smallest{ prop.m_size.z };
				if (second < smallest)
					std::swap(second, smallest);
				if (largest < second)
					std::swap(largest, second);

				params.m_uvScaleU = largest / def.m_textureTile;
				params.m_uvScaleV = second / def.m_textureTile;
			}

			factory.create(params);
		}
	}
} // namespace game::factory
