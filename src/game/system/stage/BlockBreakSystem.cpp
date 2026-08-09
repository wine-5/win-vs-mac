#include "BlockBreakSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/stage/DestructibleComponent.h"
#include "game/component/stage/BlockDebrisComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/stage/ExtensionPickupComponent.h"
#include "game/constant/ExtensionIconId.h"
#include "game/event/InGameEvents.h"
#include "core/utility/Log.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace
{
	/// @brief 破片の初速（ユニット/秒）
	///
	/// 硬いものは砕けた瞬間に鋭く弾け飛ぶ。ここが遅いと、ゆっくり崩れる
	/// 柔らかいもの（スポンジ）に見える
	constexpr float BURST_SPEED{ 620.0f };

	/// @brief 破片へ加える上向きの初速の範囲（ユニット/秒）
	///
	/// 上へ飛ばしすぎると重さが消えるため、横へ弾ける勢いより控えめにする
	constexpr float BURST_LIFT_MIN{ 120.0f };
	constexpr float BURST_LIFT_MAX{ 320.0f };

	/// @brief 破片の回転速度の上限（ラジアン/秒）
	///
	/// 砕けた直後は速く回り、床に当たるたびに落ち着く（減衰はBlockDebrisSystem側）
	constexpr float SPIN_SPEED{ 16.0f };

	/// @brief モデル素材の実寸。ブロックのスケールは 実寸 / これ で決まっている
	constexpr float BASE_SIZE{ 100.0f };

	/// @brief 落ちる欠片のビルボードの大きさ（ワールド単位）
	constexpr float DROP_BILLBOARD_SIZE{ 46.0f };

	/// @brief 欠片が漂う高さ（ブロックの底面からの相対Y）
	///
	/// 床にめり込むと拾いにくいので、少し浮かせた位置で落ち着かせる
	constexpr float DROP_REST_HEIGHT{ 55.0f };

	/// @brief 欠片が飛び出す勢い（水平・ユニット/秒）
	///
	/// 複数落とすブロックで同じ場所に重ならないよう、軽く散らす
	constexpr float DROP_BURST_SPEED{ 130.0f };

	/// @brief 欠片が飛び出す勢い（上向き・ユニット/秒）
	constexpr float DROP_BURST_LIFT_MIN{ 260.0f };
	constexpr float DROP_BURST_LIFT_MAX{ 400.0f };

	/**
	 * @brief 範囲内の一様乱数を返す
	 * @param min 最小値
	 * @param max 最大値
	 * @return min〜maxの乱数
	 */
	float randomRange(float min, float max)
	{
		static std::mt19937 engine{ std::random_device{}() };
		std::uniform_real_distribution<float> distribution{ min, max };
		return distribution(engine);
	}

	/**
	 * @brief 拡張子の種別を1つ抽選する
	 *
	 * 「中身が分からない」ブロック（ZIPなど）が落とすものを決める。
	 * 実PCの拡張子ヒストグラムからドロップを決める仕組みが入るまでの暫定で、
	 * 今は全種別を等確率で引く
	 * @return 抽選した種別
	 */
	core::data::FileExtensionType randomExtensionType()
	{
		const int count{ static_cast<int>(core::data::FileExtensionType::Count) };
		const int index{ static_cast<int>(randomRange(0.0f, static_cast<float>(count))) };
		return static_cast<core::data::FileExtensionType>(std::min(index, count - 1));
	}
} // namespace

namespace game::system::stage
{
	BlockBreakSystem::BlockBreakSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::iface::IRenderer& renderer,
	    core::iface::IResourceManager& resourceManager,
	    core::base::EventBus& eventBus,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	    , m_renderer{ renderer }
	    , m_resourceManager{ resourceManager }
	    , m_eventBus{ eventBus }
	    , m_playerId{ playerId }
	{
	}

	void BlockBreakSystem::update([[maybe_unused]] float deltaTime)
	{
		// 見るのは「振り始め」ではなく「当たり判定が解決された瞬間」。
		// プレイヤーの攻撃には playerData.json の attackWindup ぶんの溜めがあり、
		// 振り始めで判定すると剣が振り下ろされる前にひびが入る
		const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(m_playerId) };
		if (attack == nullptr || !attack->m_justResolved)
			return;

		// 弾はブロックを削らない。近づいて剣を振る理由を残すため
		if (m_componentManager.has<component::combat::ProjectileComponent>(m_playerId))
			return;

		const auto* playerTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(m_playerId) };
		if (playerTransform == nullptr)
			return;

		const auto blocks{ m_componentManager.getAllEntities<component::stage::DestructibleComponent>() };
		for (const auto blockId : blocks)
		{
			const auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
			if (destructible.isBroken())
				continue;

			const auto* blockTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(blockId) };
			if (blockTransform == nullptr)
				continue;

			// 箱の表面まで届いていれば当たりとする。中心同士の距離で測ると
			// 大きなブロックほど届きにくくなり、見た目と食い違う
			float reach{ attack->m_attackRange };
			if (const auto* collider{ m_componentManager.tryGet<component::combat::ColliderComponent>(blockId) })
				reach += std::max({ collider->m_size.x, collider->m_size.z }) * 0.5f;

			const float dx{ playerTransform->m_position.x - blockTransform->m_position.x };
			const float dz{ playerTransform->m_position.z - blockTransform->m_position.z };
			if (dx * dx + dz * dz > reach * reach)
				continue;

			hitBlock(blockId);
		}
	}

	void BlockBreakSystem::hitBlock(core::ecs::EntityId blockId)
	{
		auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
		++destructible.m_hitCount;

		// ひびを1段階進める。打撃回数とひびの枚数は一致しないので、
		// 対応付けは DestructibleComponent::crackStage が受け持つ
		const int stage{ destructible.crackStage() };
		if (stage > 0 && stage < static_cast<int>(destructible.m_crackTextures.size()))
		{
			const int texture{ destructible.m_crackTextures[stage] };
			if (texture != -1)
			{
				m_renderer.setModelTexture(destructible.m_intactHandle, texture);
				// 破片側にも同じ絵を貼っておく。破壊した瞬間に絵が戻ると割れて見えない
				m_renderer.setModelTexture(destructible.m_fracturedHandle, texture);
			}
		}

		if (destructible.isBroken())
		{
			breakBlock(blockId);
			return;
		}

		// まだ壊れていない打撃。次で壊れるかを渡して「あと1回」を音で知らせられるようにする
		m_eventBus.publish(event::BlockHitEvent{ blockId,
		    destructible.m_hitCount + 1 >= destructible.m_hitsToBreak });
	}

	void BlockBreakSystem::breakBlock(core::ecs::EntityId blockId)
	{
		const auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(blockId) };

		// 見た目を「割ってあるモデル」へ差し替える。以降の描画はこちらになる
		auto* render{ m_componentManager.tryGet<component::visual::RenderComponent>(blockId) };
		if (render == nullptr || destructible.m_fracturedHandle == -1)
			return;
		render->m_modelHandle = destructible.m_fracturedHandle;

		// 壊れた瞬間から通り抜けられるようにする。当たり判定を残すと
		// 破片が飛んでいるのに見えない壁が立ったままになる
		m_componentManager.remove<component::combat::ColliderComponent>(blockId);

		component::stage::BlockDebrisComponent debris{};
		debris.m_modelScale = transform.m_scale.y;
		debris.m_floorY = transform.m_position.y - transform.m_scale.y * BASE_SIZE * 0.5f;

		const int frameCount{ m_renderer.getModelFrameCount(destructible.m_fracturedHandle) };
		debris.m_fragments.reserve(static_cast<std::size_t>(frameCount));

		for (int i{ 0 }; i < frameCount; ++i)
		{
			component::stage::BlockDebrisFragment fragment{};
			fragment.m_frameIndex = i;
			fragment.m_pivot = m_renderer.getModelFrameCenter(destructible.m_fracturedHandle, i);

			// 破片の重心はモデルのローカル座標なので、ブロックを置いた場所へ移す
			fragment.m_position = transform.m_position + fragment.m_pivot * debris.m_modelScale;

			// ブロックの中心から外向きに弾く。中心にぴったり重なった破片は
			// 向きが決まらないので上へ逃がす
			core::Vector3 direction{ fragment.m_position - transform.m_position };
			if (direction.lengthSq() <= 0.0f)
				direction = { 0.0f, 1.0f, 0.0f };

			fragment.m_velocity = direction.normalized() * (BURST_SPEED * randomRange(0.6f, 1.3f));
			fragment.m_velocity.y += randomRange(BURST_LIFT_MIN, BURST_LIFT_MAX);
			fragment.m_angular = {
				randomRange(-SPIN_SPEED, SPIN_SPEED),
				randomRange(-SPIN_SPEED, SPIN_SPEED),
				randomRange(-SPIN_SPEED, SPIN_SPEED)
			};
			debris.m_fragments.push_back(fragment);
		}

		m_componentManager.add<component::stage::BlockDebrisComponent>(blockId, debris);

		spawnDrops(blockId);

		m_eventBus.publish(event::BlockBrokenEvent{ blockId, transform.m_position });
	}

	void BlockBreakSystem::spawnDrops(core::ecs::EntityId blockId)
	{
		const auto& destructible{ m_componentManager.get<component::stage::DestructibleComponent>(blockId) };
		if (destructible.m_dropCount <= 0)
			return;

		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(blockId) };
		const float restY{ transform.m_position.y - transform.m_scale.y * BASE_SIZE * 0.5f + DROP_REST_HEIGHT };

		for (int i{ 0 }; i < destructible.m_dropCount; ++i)
		{
			// 抽選するブロック（ZIPなど）は1個ごとに種別を引く。
			// まとめて引くと3個とも同じ拡張子になり、中身が詰まっている感じが出ない
			const auto type{ destructible.m_isDropRandom ? randomExtensionType() : destructible.m_dropType };

			core::ecs::Entity entity{ m_entityManager.create() };
			const auto dropId{ entity.getId() };

			component::movement::TransformComponent dropTransform{};
			dropTransform.m_position = transform.m_position;
			m_componentManager.add<component::movement::TransformComponent>(dropId, dropTransform);

			// 見た目は装備スロットと同じ拡張子アイコン。
			// 「壊したブロックの絵」「落ちた欠片の絵」「スロットに入る絵」が揃う
			component::visual::RenderComponent render{};
			render.m_billboardImage = m_resourceManager.loadImageById(constant::toExtensionIconId(type));
			render.m_billboardSize = DROP_BILLBOARD_SIZE;
			m_componentManager.add<component::visual::RenderComponent>(dropId, render);

			component::stage::ExtensionPickupComponent pickup{};
			pickup.m_type = type;
			pickup.m_restY = restY;
			pickup.m_velocity = {
				randomRange(-DROP_BURST_SPEED, DROP_BURST_SPEED),
				randomRange(DROP_BURST_LIFT_MIN, DROP_BURST_LIFT_MAX),
				randomRange(-DROP_BURST_SPEED, DROP_BURST_SPEED)
			};
			m_componentManager.add<component::stage::ExtensionPickupComponent>(dropId, pickup);
		}
	}
} // namespace game::system::stage
