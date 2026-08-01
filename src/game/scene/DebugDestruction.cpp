#include "DebugDestruction.h"
#include "core/base/ServiceLocator.h"
#include "core/input/KeyCode.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Log.h"
#include <algorithm>
#include <format>
#include <random>

namespace
{
	/// @brief 検証用のブロック（stageCatalog.json の block_zip と同じもの）
	constexpr std::string_view BLOCK_MODEL_PATH{ "assets/model/stage/BlockZip.mqo" };

	/// @brief ひび段階テクスチャのパスを組み立てる土台（gen_crack_textures.py の出力に合わせる）
	constexpr std::string_view BLOCK_TEXTURE_BASE{ "assets/model/stage/BlockZip" };

	/// @brief 乱数（検証用なのでこの翻訳単位に閉じたもので足りる）
	std::mt19937& rng()
	{
		static std::mt19937 engine{ std::random_device{}() };
		return engine;
	}

	/**
	 * @brief 範囲内の一様乱数を返す
	 * @param min 最小値
	 * @param max 最大値
	 * @return min〜maxの乱数
	 */
	float randomRange(float min, float max)
	{
		std::uniform_real_distribution<float> distribution{ min, max };
		return distribution(rng());
	}

	/// @brief カメラの注視点の高さ
	constexpr float CAMERA_TARGET_HEIGHT{ 90.0f };

	/// @brief カメラの距離と高さ
	constexpr float CAMERA_DISTANCE{ 620.0f };
	constexpr float CAMERA_HEIGHT{ 330.0f };

	/// @brief カメラが被写体を回り込む速さ（ラジアン/秒）
	constexpr float CAMERA_ORBIT_SPEED{ 0.18f };

	/// @brief 破片が跳ねる床の高さ
	constexpr float FLOOR_Y{ 0.0f };

	/// @brief 床で跳ね返るときに残る速度の割合
	constexpr float BOUNCE_RETENTION{ 0.34f };

	/// @brief 床との摩擦で水平速度に掛ける係数
	constexpr float FLOOR_FRICTION{ 0.72f };

	/// @brief 空気抵抗（1秒あたりに失う速度の割合）
	constexpr float AIR_DRAG{ 0.9f };

	/// @brief アイテムが浮かぶ高さと上下する幅
	constexpr float ITEM_BASE_HEIGHT{ 110.0f };
	constexpr float ITEM_BOB_AMPLITUDE{ 18.0f };

	/// @brief アイテムのスケール（モデル素材の実寸に対する比率）
	constexpr float ITEM_SCALE{ 0.35f };

	/// @brief 1回の取得で増える攻撃力
	constexpr int ATTACK_GAIN{ 5 };
} // namespace

namespace game::scene
{
	DebugDestruction::DebugDestruction(core::iface::ICamera& camera,
	    core::iface::IRenderer& renderer,
	    core::iface::IResourceManager& resourceManager,
	    core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_camera{ camera }
	    , m_renderer{ renderer }
	    , m_resourceManager{ resourceManager }
	    , m_inputProvider{ inputProvider }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
		m_screen.setBackgroundColor(10, 15, 22);
		buildFragments();
		loadCrackTextures();

		const int source{ m_resourceManager.loadModelByPath(BLOCK_MODEL_PATH) };
		m_itemHandle = m_resourceManager.duplicateModel(source);

		core::log::info("DebugDestruction: 破片 {} 個を生成しました", m_fragments.size());
	}

	DebugDestruction::~DebugDestruction()
	{
		// 複製ハンドルの見た目を元へ戻す（プールへ返る場合に赤いまま残るのを防ぐ）
		for (const auto& fragment : m_fragments)
			m_renderer.resetModelAppearance(fragment.m_modelHandle);
	}

	void DebugDestruction::buildFragments()
	{
		const int source{ m_resourceManager.loadModelByPath(BLOCK_MODEL_PATH) };
		if (source == -1)
		{
			core::log::error("DebugDestruction: ブロックモデルの読み込みに失敗しました");
			return;
		}

		// ブロックをGRID^3の格子に分け、そのマス目の中心へ小さな立方体を置く。
		// 破片モデルを作らずに済むので、あらかじめ割ったmqoが無い段階でも試せる
		const float cellSize{ BLOCK_SIZE / GRID };
		const float origin{ -BLOCK_SIZE * 0.5f + cellSize * 0.5f };

		m_fragments.reserve(static_cast<std::size_t>(GRID) * GRID * GRID);
		for (int ix{ 0 }; ix < GRID; ++ix)
		{
			for (int iy{ 0 }; iy < GRID; ++iy)
			{
				for (int iz{ 0 }; iz < GRID; ++iz)
				{
					Fragment fragment{};
					fragment.m_modelHandle = m_resourceManager.duplicateModel(source);
					fragment.m_home = {
						origin + ix * cellSize,
						// ブロックの底面を床に合わせる（中心ではなく足元を原点にする）
						FLOOR_Y + BLOCK_SIZE * 0.5f + origin + iy * cellSize,
						origin + iz * cellSize
					};
					fragment.m_position = fragment.m_home;
					m_fragments.push_back(fragment);
				}
			}
		}
	}

	void DebugDestruction::loadCrackTextures()
	{
		// [0] は無傷。リセット時に元の絵へ戻すため、段階0も配列に入れておく
		m_crackTextures.push_back(
		    m_resourceManager.loadImageByPath(std::string(BLOCK_TEXTURE_BASE) + ".png"));

		for (int stage{ 1 }; stage <= CRACK_STAGE_COUNT; ++stage)
		{
			m_crackTextures.push_back(m_resourceManager.loadImageByPath(
			    std::format("{}_crack{}.png", BLOCK_TEXTURE_BASE, stage)));
		}
	}

	void DebugDestruction::applyCrackStage()
	{
		if (m_crackTextures.empty())
			return;

		// 最終打撃は破壊なので、テクスチャは最後のひび段階で頭打ちにする
		const std::size_t index{ std::min(static_cast<std::size_t>(m_hitCount),
			m_crackTextures.size() - 1) };
		const int texture{ m_crackTextures[index] };
		if (texture == -1)
			return;

		// 破片はそれぞれ複製ハンドルなので、1つずつ貼り替える必要がある。
		// 複製元へ貼ると同じモデルを使う他のブロックまでひび割れてしまう
		for (const auto& fragment : m_fragments)
			m_renderer.setModelTexture(fragment.m_modelHandle, texture);
	}

	void DebugDestruction::update(float deltaTime)
	{
		m_phaseTime += deltaTime;
		m_cameraAngle += deltaTime * CAMERA_ORBIT_SPEED;
		m_shake = std::max(0.0f, m_shake - deltaTime * 3.0f);
		m_gainFlash = std::max(0.0f, m_gainFlash - deltaTime * 1.6f);
		m_itemSpin += deltaTime * 2.4f;

		// 疑似プレイヤーの移動（アイテムへ近づくため）
		core::Vector3 move{};
		if (m_inputProvider.isKeyDown(core::input::KeyCode::W))
			move.z += 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::S))
			move.z -= 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::A))
			move.x -= 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::D))
			move.x += 1.0f;
		if (move.lengthSq() > 0.0f)
			m_playerPosition += move.normalized() * (PLAYER_SPEED * deltaTime);

		if (m_inputProvider.isKeyPressed(core::input::KeyCode::Enter))
			reset();

		switch (m_phase)
		{
		case Phase::Intact:
			if (m_inputProvider.isKeyPressed(core::input::KeyCode::Space))
				hit();
			break;

		case Phase::Broken:
			updateFragments(deltaTime);
			// 破片が消え終わったらアイテムを出す
			if (m_phaseTime > FRAGMENT_LIFE)
			{
				m_phase = Phase::Dropped;
				m_phaseTime = 0.0f;
				m_itemPosition = { 0.0f, ITEM_BASE_HEIGHT, 0.0f };
			}
			break;

		case Phase::Dropped:
			updateItem();
			break;

		case Phase::Gained:
			break;
		}
	}

	void DebugDestruction::hit()
	{
		++m_hitCount;
		m_shake = 0.35f;
		applyCrackStage();
		if (m_hitCount >= HITS_TO_BREAK)
			explode();
	}

	void DebugDestruction::explode()
	{
		m_phase = Phase::Broken;
		m_phaseTime = 0.0f;
		m_shake = 1.0f;

		// ブロックの中心から外向きに弾く
		const core::Vector3 center{ 0.0f, FLOOR_Y + BLOCK_SIZE * 0.5f, 0.0f };
		for (auto& fragment : m_fragments)
		{
			core::Vector3 direction{ fragment.m_home - center };
			direction += { randomRange(-20.0f, 20.0f), 0.0f, randomRange(-20.0f, 20.0f) };
			if (direction.lengthSq() <= 0.0f)
				direction = { 0.0f, 1.0f, 0.0f };

			const float power{ BURST_SPEED * randomRange(0.6f, 1.3f) };
			fragment.m_velocity = direction.normalized() * power;
			fragment.m_velocity.y += randomRange(180.0f, 460.0f);
			fragment.m_angular = {
				randomRange(-8.0f, 8.0f),
				randomRange(-8.0f, 8.0f),
				randomRange(-8.0f, 8.0f)
			};
		}
	}

	void DebugDestruction::updateFragments(float deltaTime)
	{
		for (auto& fragment : m_fragments)
		{
			fragment.m_position += fragment.m_velocity * deltaTime;
			fragment.m_velocity.y -= GRAVITY * deltaTime;
			fragment.m_velocity = fragment.m_velocity * (1.0f - std::min(0.9f, AIR_DRAG * deltaTime));
			fragment.m_rotation += fragment.m_angular * deltaTime;

			if (fragment.m_position.y < FLOOR_Y)
			{
				fragment.m_position.y = FLOOR_Y;
				fragment.m_velocity.y = -fragment.m_velocity.y * BOUNCE_RETENTION;
				fragment.m_velocity.x *= FLOOR_FRICTION;
				fragment.m_velocity.z *= FLOOR_FRICTION;
				fragment.m_angular = fragment.m_angular * 0.6f;
			}

			// 後半でフェードしながら縮む。データが消えていくように見せる
			const float fadeStart{ FRAGMENT_LIFE * 0.5f };
			const float fade{ std::clamp((m_phaseTime - fadeStart) / (FRAGMENT_LIFE - fadeStart), 0.0f, 1.0f) };
			fragment.m_alpha = 1.0f - fade;
			fragment.m_scale = 1.0f - fade * 0.85f;
		}
	}

	void DebugDestruction::updateItem()
	{
		m_itemPosition.y = ITEM_BASE_HEIGHT + std::sin(m_phaseTime * 2.2f) * ITEM_BOB_AMPLITUDE;

		const core::Vector3 toPlayer{ m_playerPosition - m_itemPosition };
		const float distanceSq{ toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z };
		if (distanceSq > PICKUP_RANGE * PICKUP_RANGE)
			return;

		// 取得。ここでパラメータが変わる
		m_attack += ATTACK_GAIN;
		m_gainedAttack += ATTACK_GAIN;
		m_gainFlash = 1.0f;
		m_phase = Phase::Gained;
		m_phaseTime = 0.0f;
		core::log::info("DebugDestruction: アイテムを取得しました（攻撃力 {}）", m_attack);
	}

	void DebugDestruction::reset()
	{
		for (auto& fragment : m_fragments)
		{
			fragment.m_position = fragment.m_home;
			fragment.m_velocity = {};
			fragment.m_rotation = {};
			fragment.m_angular = {};
			fragment.m_scale = 1.0f;
			fragment.m_alpha = 1.0f;
			m_renderer.resetModelAppearance(fragment.m_modelHandle);
		}
		m_phase = Phase::Intact;
		m_hitCount = 0;
		m_phaseTime = 0.0f;
		m_shake = 0.0f;
		applyCrackStage(); // 無傷のテクスチャへ戻す
	}

	void DebugDestruction::draw()
	{
		// カメラを回り込ませ、打撃時は揺らす
		const float shakeOffset{ m_shake * m_shake * 60.0f * std::sin(m_phaseTime * 60.0f) };
		const core::Vector3 eye{
			std::sin(m_cameraAngle) * CAMERA_DISTANCE,
			CAMERA_HEIGHT + shakeOffset,
			std::cos(m_cameraAngle) * CAMERA_DISTANCE
		};
		m_camera.setLookAt(eye, { 0.0f, CAMERA_TARGET_HEIGHT, 0.0f });

		const float cellScale{ (BLOCK_SIZE / GRID) / BASE_SIZE };

		// ダメージ段階では破片をわずかに外へ押し出し、隙間を「ひび」として見せる
		const float gap{ m_phase == Phase::Intact ? m_hitCount * 2.6f : 0.0f };
		const core::Vector3 center{ 0.0f, FLOOR_Y + BLOCK_SIZE * 0.5f, 0.0f };

		for (const auto& fragment : m_fragments)
		{
			if (fragment.m_alpha <= 0.01f || fragment.m_scale <= 0.01f)
				continue;

			core::Vector3 position{ fragment.m_position };
			if (gap > 0.0f)
			{
				const core::Vector3 outward{ fragment.m_home - center };
				if (outward.lengthSq() > 0.0f)
					position += outward.normalized() * gap;
			}

			// 破壊中は赤熱させつつフェードさせる（破断面の代わり）
			if (m_phase == Phase::Broken)
				m_renderer.applyDeathDissolve(fragment.m_modelHandle, 1.0f - fragment.m_alpha, fragment.m_alpha);

			m_renderer.drawModel(fragment.m_modelHandle, position, fragment.m_rotation,
			    { cellScale * fragment.m_scale, cellScale * fragment.m_scale, cellScale * fragment.m_scale });
		}

		// アイテムと取得範囲
		if (m_phase == Phase::Dropped)
		{
			m_renderer.drawModel(m_itemHandle, m_itemPosition, { 0.0f, m_itemSpin, 0.4f },
			    { ITEM_SCALE, ITEM_SCALE, ITEM_SCALE });
			m_renderer.drawGroundCircle({ m_itemPosition.x, FLOOR_Y + 1.0f, m_itemPosition.z },
			    PICKUP_RANGE, 0x6022D3EEu, false);
		}

		// 疑似プレイヤーの居場所
		m_renderer.drawDebugSphere({ m_playerPosition.x, FLOOR_Y + 40.0f, m_playerPosition.z },
		    40.0f, 0xFF4ADE80u);

		drawHud();
	}

	void DebugDestruction::drawHud()
	{
		constexpr int LINE_HEIGHT{ 24 };
		constexpr int FONT_SIZE{ 18 };
		constexpr unsigned int TEXT_COLOR{ 0xFFDBE6F0u };
		constexpr unsigned int ACCENT_COLOR{ 0xFF22D3EEu };

		// DxLibの描画はShift_JISを期待するため、UTF-8のソース文字列をそのまま渡すと化ける
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };

		int y{ 24 };
		const auto line = [&](const std::string& text, unsigned int color)
		{
			const std::string drawable{ converter ? converter->utf8ToShiftJis(text) : text };
			m_uiRenderer.drawText(24, y, drawable.c_str(), color, FONT_SIZE);
			y += LINE_HEIGHT;
		};

		line("DEBUG: 破壊検証シーン", ACCENT_COLOR);
		line(std::format("破片 {} 個 / 描画コール {}",
		         m_fragments.size(), m_renderer.getDrawCallCount()),
		    TEXT_COLOR);

		switch (m_phase)
		{
		case Phase::Intact:
			line(std::format("ダメージ {} / {}  [Space で殴る]", m_hitCount, HITS_TO_BREAK), TEXT_COLOR);
			break;
		case Phase::Broken:
			line("破壊中...", 0xFFF59E0Bu);
			break;
		case Phase::Dropped:
			line("アイテム出現  [WASD で近づく]", 0xFFF59E0Bu);
			break;
		case Phase::Gained:
			line("取得！", 0xFF4ADE80u);
			break;
		}

		// パラメータ表示。取得直後は色を変えて変化を目立たせる
		const unsigned int attackColor{ m_gainFlash > 0.0f ? 0xFF4ADE80u : TEXT_COLOR };
		line(std::format("攻撃力 {}  (+{})", m_attack, m_gainedAttack), attackColor);
		line("[Enter] リセット", 0xFF7F93A8u);
	}
} // namespace game::scene
