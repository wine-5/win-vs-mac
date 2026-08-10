#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/utility/MathConstants.h"
#include "core/utility/Rotation.h"
#include "core/data/PropDefinition.h"
#include "game/constant/ModelId.h"
#include "game/constant/PropCollision.h"
#include "game/constant/PropId.h"
#include "game/constant/PropRole.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/visual/LightComponent.h"
#include "game/component/stage/BossGateComponent.h"
#include "game/component/stage/RenameTerminalComponent.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace
{
	// ボス扉を開状態で沈める際、自分の高さに加えて余分に下げる量。
	// 床の厚みぶん余計に沈めて、閉じる前に上端がのぞかないようにする
	constexpr float BOSS_GATE_SINK_MARGIN{ 100.0f };

	// 用意されているひび段階の数（tools/gen_crack_textures.py の STAGE_COUNT と合わせる）
	constexpr int CRACK_STAGE_COUNT{ 3 };

	// stageCatalog.json の dropExtension に書くと「種別を抽選する」意味になる綴り
	constexpr std::string_view RANDOM_DROP_KEY{ "random" };

	/**
	 * @brief 傾けた配置物がY方向に占める高さを求める
	 *
	 * 薄い床でも坂として寝かせれば見た目の高さは実寸のYを大きく超える。
	 * 沈める量をsize.yで決めると坂だけ床から突き出てしまうため、
	 * 三辺を回転させてY成分を足し合わせた「傾きこみの高さ」を使う。
	 * @param size 配置物の実寸
	 * @param rotation オイラー角（ラジアン）
	 * @return 回転後にY方向へ占める高さ
	 */
	float rotatedHeight(const core::Vector3& size, const core::Vector3& rotation) noexcept
	{
		const float x{ core::utility::rotateEulerXYZ(core::Vector3{ size.x, 0.0f, 0.0f }, rotation).y };
		const float y{ core::utility::rotateEulerXYZ(core::Vector3{ 0.0f, size.y, 0.0f }, rotation).y };
		const float z{ core::utility::rotateEulerXYZ(core::Vector3{ 0.0f, 0.0f, size.z }, rotation).y };
		return std::abs(x) + std::abs(y) + std::abs(z);
	}

	/**
	 * @brief 抽選対象の種類IDを解決する
	 *
	 * block_random は「壊せるブロックがある」という配置だけを表し、
	 * 実際の中身はプレイのたびに抽選で決まる。それ以外の種類はそのまま返す。
	 * @param type ステージ配置に書かれた種類ID
	 * @param table 抽選表（stageCatalog.jsonのblockTable）
	 * @param rng 乱数エンジン
	 * @return 実際に生成する種類ID
	 */
	std::string resolvePropType(const std::string& type,
	    const core::data::BlockTable& table,
	    std::mt19937& rng)
	{
		if (type != game::constant::prop_id::BLOCK_RANDOM)
			return type;

		const float total{ table.totalWeight() };
		if (total <= 0.0f)
		{
			core::log::error("blockTableに抽選できる行がありません（block_randomを解決できない）");
			return type;
		}

		std::uniform_real_distribution<float> distribution{ 0.0f, total };
		return std::string(table.pick(distribution(rng)));
	}

	/**
	 * @brief モデルのパスから拡張子を取り除いた土台を返す
	 *
	 * 割ったモデルもひびテクスチャも「元のモデル名 + 決まった接尾辞」で導ける。
	 * ブロックが増えるたびにJSONへパスを3種類書き足すのは保守が割に合わないため、
	 * 命名規約から組み立てる（生成側は tools/gen_fracture_models.py と
	 * tools/gen_crack_textures.py が同じ規約で吐く）
	 * @param modelPath モデルのパス（例: assets/model/stage/BlockZip.mqo）
	 * @return 拡張子を除いたパス（例: assets/model/stage/BlockZip）
	 */
	std::string stripExtension(const std::string& modelPath)
	{
		const auto dot{ modelPath.find_last_of('.') };
		return dot == std::string::npos ? modelPath : modelPath.substr(0, dot);
	}
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

	void FactoryInitializer::initializePlayer(const data::PlayerData& playerData,
	    const component::combat::PlayerStatBaseComponent& statBase)
	{
		int playerHandle{m_resourceManager.loadModelById(constant::model_id::PLAYER)};

		// プレイヤーモデルはハンドルがキャッシュされ、再プレイでも同じ実体を使い回す。
		// 前回プレイの死亡（Dying）アニメがアタッチされたまま残ると、新しいIdleと重なって
		// 死亡ポーズが抜けないため、生成前に一度すべてのアニメをデタッチして初期化する。
		// 敵は複製ハンドル＋プール返却時のデタッチで済むが、プレイヤーは複製しないのでここで行う
		m_resourceManager.detachAllAnimations(playerHandle);

		m_factoryManager.getPlayerFactory().create(playerHandle, playerData, statBase);
	}

	void FactoryInitializer::initializeProps()
	{
		const auto& stage{ m_resourceManager.getStageMetadata() };
		auto& factory{ m_factoryManager.getStagePropFactory() };

		// ブロックの中身はプレイのたびに変える。同じ席で毎回同じ物が出ると
		// 配置を覚えたプレイヤーにとってダンジョンが一本道の作業になるため
		const auto& blockTable{ m_resourceManager.getBlockTable() };
		std::mt19937 rng{ std::random_device{}() };

		for (const auto& prop : stage.m_props)
		{
			const std::string type{ resolvePropType(prop.m_type, blockTable, rng) };
			const auto& def{ m_resourceManager.getPropDefinition(type) };

			// loadModelByPath はパス単位でハンドルを使い回すため、同じ種類の配置物は
			// 全部が同じモデルを指す。静止した床・壁ならそれでよいが、壊せるブロックは
			// 1個ずつ見た目が変わる（ひび・破片）ので、複製して個別のハンドルを持たせる。
			const int sharedHandle{ m_resourceManager.loadModelByPath(def.m_modelPath) };
			const bool isDestructible{ def.m_hitsToBreak > 0 };
			const int handle{ isDestructible ? m_resourceManager.duplicateModel(sharedHandle) : sharedHandle };

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

			if (isDestructible)
			{
				const std::string base{ stripExtension(def.m_modelPath) };

				params.m_hitsToBreak = def.m_hitsToBreak;
				// 割ったモデルも1個ずつ複製する。共有すると1個壊した瞬間に
				// 同じ種類のブロック全部の破片が同じ動きで飛ぶ
				params.m_fracturedHandle = m_resourceManager.duplicateModel(
				    m_resourceManager.loadModelByPath(base + "Fractured.mqo"));

				// [0]は無傷。ひびを戻す必要は無いが、段階0を配列に入れておくと
				// 「段階＝添字」で引けて取り違えが起きない
				params.m_crackTextures.push_back(m_resourceManager.loadImageByPath(base + ".png"));
				for (int stage{ 1 }; stage <= CRACK_STAGE_COUNT; ++stage)
				{
					params.m_crackTextures.push_back(
					    m_resourceManager.loadImageByPath(base + "_crack" + std::to_string(stage) + ".png"));
				}

				// "random" は「壊すまで中身が分からない」ブロック用の特別な指定。
				// 種別は落とす瞬間に1個ずつ抽選する
				params.m_dropCount = def.m_dropCount;
				params.m_isDropRandom = def.m_dropExtension == RANDOM_DROP_KEY;
				if (!params.m_isDropRandom)
					params.m_dropType = core::data::toExtensionType(def.m_dropExtension);
			}

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
				// テクスチャ1枚がtextureTileぶんの実寸を受け持つので、速さ÷1枚の実寸が
				// そのまま「1秒あたり何枚ぶん流すか」になる。
				// JSONへ別々に書くと片方だけ直したときに見た目と力が食い違うため、ここで揃える。
				// 符号が負なのは、天面のV軸がローカル+Zと逆向き（V=0がZ+側）に貼られているため。
				// 正のまま渡すと模様だけが運ぶ向きと反対へ流れる
				if (def.m_conveyorSpeed != 0.0f && def.m_textureTile > 0.0f)
					params.m_scrollSpeedV = -def.m_conveyorSpeed / def.m_textureTile;
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
			// 沈める量は傾きこみの高さぶん＋余白で、隙間から見えないようにする。
			// 寝かせた坂も同じ役割で扱えるよう、実寸のYではなく回転後の高さで測る
			const bool isBossGate{ def.m_role == constant::prop_role::BOSS_GATE };
			if (isBossGate)
				params.m_position.y -= rotatedHeight(prop.m_size, rotation) + BOSS_GATE_SINK_MARGIN;

			const auto propId{ factory.create(params) };

			if (isBossGate)
			{
				component::stage::BossGateComponent gate{};
				gate.m_closedY = prop.m_position.y;
				gate.m_openY = params.m_position.y;
				m_componentManager.add<component::stage::BossGateComponent>(propId, gate);
			}

			// 拡張子の付け替え端末。壊せない設置物なので、破壊まわりの設定は持たない
			if (def.m_role == constant::prop_role::RENAME_TERMINAL)
			{
				m_componentManager.add<component::stage::RenameTerminalComponent>(
				    propId, component::stage::RenameTerminalComponent{});
			}
		}
	}
} // namespace game::factory
