#include "BackgroundParticleSystem.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Log.h"
#include "game/component/movement/TransformComponent.h"
#include <algorithm>
#include <cmath>
#include <array>
#include <random>

namespace
{
	constexpr const char* GLOW_IMAGE_ID{ "sky-glow" };
	constexpr const char* FOLDER_IMAGE_ID{ "sky-folder" };

	// 転送中のファイルに使う絵。セレクト画面と同じ拡張子アイコンを流用する
	constexpr std::array<const char*, 5> FILE_IMAGE_IDS{
		"ext-exe", "ext-doc", "ext-img", "ext-aud", "ext-arc"
	};

	// 空はプレイヤーの上空を中心とする。追従させることで、
	// どこまで進んでも空が付いてくる（実際の空と同じ振る舞い）
	constexpr float SKY_CENTER_HEIGHT{ 2500.0f };

	// DxLibはビルボード1枚がドローコール1回になるため、数は絞って大きめに描く
	constexpr int STAR_COUNT{ 220 };

	// カメラのファークリップは20000（InGame::NEAR_CLIP/FAR_CLIP）。
	// これを超える位置に置くと1つも描画されないため、必ず内側に収める
	constexpr float STAR_DISTANCE_MIN{ 12000.0f };
	constexpr float STAR_DISTANCE_MAX{ 17500.0f };

	constexpr float STAR_SIZE_MIN{ 100.0f };
	constexpr float STAR_SIZE_MAX{ 580.0f };
	constexpr float STAR_TWINKLE_SPEED_MIN{ 0.4f };
	constexpr float STAR_TWINKLE_SPEED_MAX{ 2.6f };
	constexpr int STAR_BRIGHTNESS_MIN{ 90 };
	constexpr int STAR_BRIGHTNESS_MAX{ 200 };

	// 星は空を見上げたときに見えてほしいので、水平より下にはほとんど置かない
	constexpr float STAR_LOWEST_HEIGHT_RATIO{ -0.08f };

	constexpr float TWO_PI{ 6.283185f };
	constexpr float PI{ 3.141593f };

	// ---- フォルダ間のファイル転送 ----
	// 空のあちこちに置き、どちらを向いても1組は目に入るようにする
	constexpr int TRANSFER_COUNT{ 3 };
	constexpr int FILES_PER_TRANSFER{ 7 };
	constexpr float TRANSFER_DISTANCE{ 13000.0f };   // 空の中心からフォルダまでの距離
	constexpr float TRANSFER_ELEVATION_MIN{ 0.16f }; // 仰角（ラジアン）。低すぎると地形に隠れる
	constexpr float TRANSFER_ELEVATION_MAX{ 0.52f };
	constexpr float TRANSFER_HALF_SPAN{ 0.16f };   // 2つのフォルダの開き（ラジアン）
	constexpr float TRANSFER_ARC_HEIGHT{ 900.0f }; // ファイルが描く弧の高さ
	constexpr float FILE_SPEED_MIN{ 0.16f };       // 進行度／秒
	constexpr float FILE_SPEED_MAX{ 0.24f };
	constexpr float FOLDER_SIZE{ 1500.0f };
	constexpr float FILE_SIZE{ 620.0f };

	/**
	 * @brief 0.0〜1.0の乱数を返す
	 * @param rng 乱数エンジン
	 * @return 一様乱数
	 */
	float randomUnit(std::mt19937& rng)
	{
		return std::uniform_real_distribution<float>{ 0.0f, 1.0f }(rng);
	}

	/**
	 * @brief 指定範囲の乱数を返す
	 * @param rng 乱数エンジン
	 * @param min 最小値
	 * @param max 最大値
	 * @return 範囲内の一様乱数
	 */
	float randomRange(std::mt19937& rng, float min, float max)
	{
		return std::uniform_real_distribution<float>{ min, max }(rng);
	}

	/**
	 * @brief ベクトルを正規化する
	 * @param v 対象のベクトル
	 * @return 長さ1のベクトル（長さ0なら上向き）
	 */
	core::Vector3 normalize(const core::Vector3& v)
	{
		const float length{ std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z) };
		if (length <= 0.0f)
			return core::Vector3{ 0.0f, 1.0f, 0.0f };
		return core::Vector3{ v.x / length, v.y / length, v.z / length };
	}
} // namespace

namespace game::system::visual
{
	BackgroundParticleSystem::BackgroundParticleSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId,
	    core::iface::IRenderer& renderer,
	    core::iface::IResourceManager& resourceManager)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	    , m_renderer{ renderer }
	{
		m_glowHandle = resourceManager.loadImageById(GLOW_IMAGE_ID);
		if (m_glowHandle == -1)
			core::log::error("空の光の画像 '{}' の読み込みに失敗しました", GLOW_IMAGE_ID);

		m_folderHandle = resourceManager.loadImageById(FOLDER_IMAGE_ID);
		if (m_folderHandle == -1)
			core::log::error("空のフォルダ画像 '{}' の読み込みに失敗しました", FOLDER_IMAGE_ID);

		m_stars.resize(STAR_COUNT);
		for (auto& star : m_stars)
			placeStar(star);

		// 転送に使うファイルの絵を集める。1枚も読めなければ転送は出さない
		std::vector<int> fileHandles{};
		for (const auto* imageId : FILE_IMAGE_IDS)
		{
			const int handle{ resourceManager.loadImageById(imageId) };
			if (handle != -1)
				fileHandles.push_back(handle);
		}
		if (fileHandles.empty() || m_folderHandle == -1)
			return;

		m_transfers.resize(TRANSFER_COUNT);
		for (int i{ 0 }; i < TRANSFER_COUNT; ++i)
		{
			// 方角を等分して置く。どちらを向いても1組は視界に入りやすくする
			placeTransfer(m_transfers[i], TWO_PI * i / TRANSFER_COUNT + randomRange(m_rng, -0.3f, 0.3f));

			auto& files{ m_transfers[i].m_files };
			files.resize(FILES_PER_TRANSFER);
			for (int f{ 0 }; f < FILES_PER_TRANSFER; ++f)
			{
				// 進行度を等間隔にずらし、途切れない列にする
				files[f].m_progress = static_cast<float>(f) / FILES_PER_TRANSFER;
				files[f].m_speed = randomRange(m_rng, FILE_SPEED_MIN, FILE_SPEED_MAX);
				files[f].m_spin = randomRange(m_rng, -0.4f, 0.4f);
				files[f].m_spinSpeed = randomRange(m_rng, -1.2f, 1.2f);
				files[f].m_imageHandle = fileHandles[std::uniform_int_distribution<std::size_t>{ 0, fileHandles.size() - 1 }(m_rng)];
			}
		}
	}

	void BackgroundParticleSystem::placeTransfer(FileTransfer& transfer, float azimuth)
	{
		const float elevation{ randomRange(m_rng, TRANSFER_ELEVATION_MIN, TRANSFER_ELEVATION_MAX) };

		// 同じ仰角のまま方位だけずらして2つ置く。並んで見えるので
		// 「AからBへ渡している」という関係が読み取りやすい
		auto toPosition = [&](float az, float el)
		{
			const float horizontal{ std::cos(el) * TRANSFER_DISTANCE };
			return core::Vector3{
				std::cos(az) * horizontal,
				std::sin(el) * TRANSFER_DISTANCE,
				std::sin(az) * horizontal
			};
		};

		transfer.m_fromPosition = toPosition(azimuth - TRANSFER_HALF_SPAN, elevation);
		transfer.m_toPosition = toPosition(azimuth + TRANSFER_HALF_SPAN, elevation);
	}

	core::Vector3 BackgroundParticleSystem::computeFilePosition(const FileTransfer& transfer,
	    float progress, const core::Vector3& center) const
	{
		// 直線ではなく弧を描かせる。Windowsのコピー中アニメーションと同じ動きで、
		// 「投げ渡している」ことが直感的に分かる
		const float arc{ std::sin(progress * PI) * TRANSFER_ARC_HEIGHT };
		return core::Vector3{
			center.x + transfer.m_fromPosition.x + (transfer.m_toPosition.x - transfer.m_fromPosition.x) * progress,
			center.y + transfer.m_fromPosition.y + (transfer.m_toPosition.y - transfer.m_fromPosition.y) * progress + arc,
			center.z + transfer.m_fromPosition.z + (transfer.m_toPosition.z - transfer.m_fromPosition.z) * progress
		};
	}

	core::Vector3 BackgroundParticleSystem::getSkyCenter() const
	{
		core::Vector3 center{ 0.0f, SKY_CENTER_HEIGHT, 0.0f };
		if (m_componentManager.has<component::movement::TransformComponent>(m_playerId))
		{
			const auto& position{ m_componentManager.get<component::movement::TransformComponent>(m_playerId).m_position };
			center.x = position.x;
			center.y = position.y + SKY_CENTER_HEIGHT;
			center.z = position.z;
		}
		return center;
	}

	void BackgroundParticleSystem::placeStar(Star& star)
	{
		// 上半球寄りの球面へばら撒く。yを下限で切ることで、見上げたときに空が埋まる
		const float theta{ randomRange(m_rng, 0.0f, TWO_PI) };
		const float height{ randomRange(m_rng, STAR_LOWEST_HEIGHT_RATIO, 1.0f) };
		const float radius{ std::sqrt(std::max(0.0f, 1.0f - height * height)) };

		star.m_direction = normalize(core::Vector3{ radius * std::cos(theta), height, radius * std::sin(theta) });
		star.m_distance = randomRange(m_rng, STAR_DISTANCE_MIN, STAR_DISTANCE_MAX);

		// 小さい星を多く、大きい星をまれにする（3つ掛けて分布を小さい側へ寄せる）
		const float sizeBias{ randomUnit(m_rng) * randomUnit(m_rng) * randomUnit(m_rng) };
		star.m_size = STAR_SIZE_MIN + (STAR_SIZE_MAX - STAR_SIZE_MIN) * sizeBias;

		star.m_twinklePhase = randomRange(m_rng, 0.0f, TWO_PI);
		star.m_twinkleSpeed = randomRange(m_rng, STAR_TWINKLE_SPEED_MIN, STAR_TWINKLE_SPEED_MAX);
		star.m_brightness = static_cast<int>(randomRange(m_rng, STAR_BRIGHTNESS_MIN, STAR_BRIGHTNESS_MAX));
	}

	void BackgroundParticleSystem::update(float deltaTime)
	{
		m_elapsedTime += deltaTime;

		for (auto& transfer : m_transfers)
		{
			for (auto& file : transfer.m_files)
			{
				// 転送先に着いたら転送元へ戻す。列が途切れず流れ続ける
				file.m_progress += file.m_speed * deltaTime;
				if (file.m_progress >= 1.0f)
					file.m_progress -= 1.0f;

				file.m_spin += file.m_spinSpeed * deltaTime;
			}
		}
	}

	void BackgroundParticleSystem::draw()
	{
		if (m_glowHandle == -1)
			return;

		const core::Vector3 center{ getSkyCenter() };

		for (const auto& star : m_stars)
		{
			// 空はプレイヤーに追従させる。視差が生まれないため、無限遠の空として振る舞う
			const core::Vector3 position{
				center.x + star.m_direction.x * star.m_distance,
				center.y + star.m_direction.y * star.m_distance,
				center.z + star.m_direction.z * star.m_distance
			};

			const core::Vector3 projected{ m_renderer.worldToScreen(position) };
			if (projected.z < 0.0f || projected.z > 1.0f)
				continue;

			// 明るさと大きさを同じ位相で揺らす。両方が同時に動くと「瞬いている」ように見える
			const float twinkle{ 0.35f + 0.65f * (0.5f + 0.5f * std::sin(m_elapsedTime * star.m_twinkleSpeed + star.m_twinklePhase)) };
			const int brightness{ static_cast<int>(star.m_brightness * twinkle) };
			if (brightness <= 0)
				continue;

			m_renderer.drawGlowBillboard(m_glowHandle, position,
			    star.m_size * (0.7f + 0.3f * twinkle), 0.0f, brightness);
		}

		drawFileTransfers(center);
	}

	void BackgroundParticleSystem::drawFileTransfers(const core::Vector3& center)
	{
		if (m_folderHandle == -1)
			return;

		for (const auto& transfer : m_transfers)
		{
			// フォルダは加算合成にしない。実体のある「入れ物」として見せたいので、
			// 絵のとおりの色で描く（加算だと黄色が飛んで白い塊になる）
			const core::Vector3 from{ center.x + transfer.m_fromPosition.x,
				center.y + transfer.m_fromPosition.y, center.z + transfer.m_fromPosition.z };
			const core::Vector3 to{ center.x + transfer.m_toPosition.x,
				center.y + transfer.m_toPosition.y, center.z + transfer.m_toPosition.z };

			m_renderer.drawBillboard(m_folderHandle, from, FOLDER_SIZE, 0.0f);
			m_renderer.drawBillboard(m_folderHandle, to, FOLDER_SIZE, 0.0f);

			for (const auto& file : transfer.m_files)
			{
				if (file.m_imageHandle == -1)
					continue;

				const core::Vector3 position{ computeFilePosition(transfer, file.m_progress, center) };
				const core::Vector3 projected{ m_renderer.worldToScreen(position) };
				if (projected.z < 0.0f || projected.z > 1.0f)
					continue;

				// 出入り口では小さく、中間で大きく。フォルダへ吸い込まれるように見える
				const float scale{ 0.45f + 0.55f * std::sin(file.m_progress * PI) };
				m_renderer.drawBillboard(file.m_imageHandle, position, FILE_SIZE * scale, file.m_spin);
			}
		}
	}
} // namespace game::system::visual
