#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/utility/MathConstants.h"
#include "core/data/PropDefinition.h"
#include "game/constant/ModelId.h"
#include "game/constant/PropCollision.h"
#include "game/constant/PropRole.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/visual/LightComponent.h"
#include "game/component/stage/BossGateComponent.h"
#include <cmath>
#include <algorithm>

namespace
{
	// ボス扉を開状態で沈める際、自分の高さに加えて余分に下げる量。
	// 床の厚みぶん余計に沈めて、閉じる前に上端がのぞかないようにする
	constexpr float BOSS_GATE_SINK_MARGIN{ 100.0f };
} // namespace

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

		// プレイヤーモデルはハンドルがキャッシュされ、再プレイでも同じ実体を使い回す。
		// 前回プレイの死亡（Dying）アニメがアタッチされたまま残ると、新しいIdleと重なって
		// 死亡ポーズが抜けないため、生成前に一度すべてのアニメをデタッチして初期化する。
		// 敵は複製ハンドル＋プール返却時のデタッチで済むが、プレイヤーは複製しないのでここで行う
		m_resourceManager.detachAllAnimations(playerHandle);

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

			params.m_scrollSpeedU = def.m_scrollU;
			params.m_scrollSpeedV = def.m_scrollV;

			const auto collision{ constant::toPropCollision(def.m_collider) };
			params.m_collision = collision;

			// Box（壁・柱・ブロック）はY回転ごとCollisionSystemが扱うので実寸をそのまま渡す。
			// Ground（床・坂）も傾きごと GroundingSystem が扱うので同じく実寸でよい
			if (collision == constant::PropCollision::Box)
				params.m_collisionSize = prop.m_size;
			else if (collision == constant::PropCollision::Ground)
			{
				params.m_collisionSize = prop.m_size;
				params.m_slideAccel = def.m_slideAccel;
				params.m_conveyorSpeed = def.m_conveyorSpeed;

				// 動く歩道は模様の流れる向き・速さを運ぶ力から導く。
				// テクスチャ1枚がtextureTileぶんの実寸を受け持ち、V軸はローカルZに沿うので、
				// 速さ÷1枚の実寸がそのまま「1秒あたり何枚ぶん流すか」になる。
				// JSONへ別々に書くと片方だけ直したときに見た目と力が食い違うため、ここで揃える
				if (def.m_conveyorSpeed != 0.0f && def.m_textureTile > 0.0f)
					params.m_scrollSpeedV = def.m_conveyorSpeed / def.m_textureTile;
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

			// ボス扉は「閉じた状態」で置かれているので、開いた状態（床下）から始める。
			// 沈める量は自分の高さぶん＋余白で、隙間から見えないようにする
			const bool isBossGate{ def.m_role == constant::prop_role::BOSS_GATE };
			if (isBossGate)
				params.m_position.y -= prop.m_size.y + BOSS_GATE_SINK_MARGIN;

			const auto propId{ factory.create(params) };

			if (isBossGate)
			{
				component::stage::BossGateComponent gate{};
				gate.m_closedY = prop.m_position.y;
				gate.m_openY = params.m_position.y;
				m_componentManager.add<component::stage::BossGateComponent>(propId, gate);
			}
		}
	}
} // namespace game::factory
