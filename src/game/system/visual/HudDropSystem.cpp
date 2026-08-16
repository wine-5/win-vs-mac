#include "HudDropSystem.h"
#include <algorithm>
#include <cmath>
#include "core/base/ServiceLocator.h"
#include "core/constant/SeType.h"
#include "core/interface/IAudioManager.h"
#include "core/utility/Easing.h"
#include "game/constant/MacAwakenTiming.h"
#include "game/event/InGameEvents.h"

namespace
{
	namespace timing = game::constant::mac_awaken;

	// HUDのレイアウト（1080p基準）。落下距離を求めるために各HUDの下端を持つ。
	// 値は各Viewの定数と揃えること（ずれると接地位置がずれる）
	struct SlotLayout
	{
		int m_bottomY; // 画面上端からHUD下端までの距離
	};

	// HudSlot の並びと同じ順に持つ
	constexpr SlotLayout SLOT_LAYOUTS[]{
		{ 132 }, // Objective … 上余白28 ＋ パネル高さ104
		{ 132 }, // Status    … 同上
		{ 344 }, // MiniMap   … 上端144 ＋ 高さ200
	};

	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// HUDごとに落ち始めをずらす間隔（秒）。同時に落とすより有機的に見える
	constexpr float SLOT_STAGGER{ 0.13f };

	// 各段階の長さ（秒）
	constexpr float RATTLE_TIME{ 0.45f };      // 震えている時間
	constexpr float FALL_TIME{ 0.45f };        // 接地するまでの落下
	constexpr float BOUNCE_UP_TIME{ 0.12f };   // 跳ね上がり
	constexpr float BOUNCE_DOWN_TIME{ 0.10f }; // 跳ねた後の落下
	constexpr float EXIT_TIME{ 0.35f };        // 画面外へ抜けるまで

	constexpr float RATTLE_END{ RATTLE_TIME };
	constexpr float FALL_END{ RATTLE_END + FALL_TIME };
	constexpr float BOUNCE_UP_END{ FALL_END + BOUNCE_UP_TIME };
	constexpr float BOUNCE_DOWN_END{ BOUNCE_UP_END + BOUNCE_DOWN_TIME };
	constexpr float EXIT_END{ BOUNCE_DOWN_END + EXIT_TIME };

	// 震えの振幅（px・1080p基準）。カメラのシェイクとは別の理由で震えていると
	// 分かるよう、細かく速い揺れにする
	constexpr int RATTLE_AMPLITUDE{ 5 };

	// 接地して跳ね上がる高さ（接地までの落下距離に対する割合）
	constexpr float BOUNCE_RATIO{ 0.18f };

} // namespace

namespace game::system::visual
{
	HudDropSystem::HudDropSystem(core::base::EventBus& eventBus, core::iface::IScreen& screen)
	    : m_screen{ screen }
	{
		// 覚醒（第2形態への移行）で落下を始める。ただし演出はカメラがボスへ寄りきってから
		// 動き出すため、実際の落下開始は update 側でズームイン時間ぶん待つ
		m_subscriptions.push_back(eventBus.subscribe<event::MacPhaseTransitionEvent>(
		    [this](const event::MacPhaseTransitionEvent&)
		    {
			    if (m_isPlaying)
				    return;
			    m_isPlaying = true;
			    m_elapsedTime = 0.0f;
			    m_hasPlayedRattleSe = false;
			    m_hasPlayedLandSe.fill(false);
		    }));
	}

	void HudDropSystem::update(float deltaTime)
	{
		if (!m_isPlaying)
			return;

		m_elapsedTime += deltaTime;

		// カメラがボスへ寄りきる（＝シェイクと衝撃波が始まる）までは何もしない
		const float localTime{ m_elapsedTime - timing::ZOOM_IN_TIME };
		if (localTime < 0.0f)
			return;

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };

		// 震え始めの音は最初の1回だけ。3つぶん鳴らすと厚くなりすぎる
		if (!m_hasPlayedRattleSe)
		{
			m_hasPlayedRattleSe = true;
			if (audio)
				audio->playSe(core::constant::SeType::HudRattle);
		}

		// 接地した瞬間に落下音を鳴らす。HUDごとにずれて落ちるので3回鳴る
		for (int i{ 0 }; i < static_cast<int>(HudSlot::Count); ++i)
		{
			const float slotTime{ localTime - SLOT_STAGGER * static_cast<float>(i) };
			if (m_hasPlayedLandSe[i] || slotTime < FALL_END)
				continue;

			m_hasPlayedLandSe[i] = true;
			if (audio)
				audio->playSe(core::constant::SeType::HudDrop);
		}
	}

	HudOffset HudDropSystem::getOffset(HudSlot slot) const
	{
		return computeOffset(static_cast<int>(slot));
	}

	bool HudDropSystem::isDropped(HudSlot slot) const
	{
		if (!m_isPlaying)
			return false;

		const float slotTime{ m_elapsedTime - timing::ZOOM_IN_TIME - SLOT_STAGGER * static_cast<float>(slot) };
		return slotTime >= EXIT_END;
	}

	float HudDropSystem::landingDistance(int slotIndex) const
	{
		// 画面の高さに合わせて拡縮する（各Viewが1080p基準で組まれているのに合わせる）
		const float scale{ static_cast<float>(m_screen.getHeight()) / static_cast<float>(BASE_SCREEN_HEIGHT) };
		const float bottomY{ static_cast<float>(SLOT_LAYOUTS[slotIndex].m_bottomY) * scale };

		// 下端が画面下端に触れるまで落とす。ここで跳ねさせると全体が見えている状態で接地する
		return static_cast<float>(m_screen.getHeight()) - bottomY;
	}

	HudOffset HudDropSystem::computeOffset(int slotIndex) const
	{
		if (!m_isPlaying)
			return {};

		const float slotTime{ m_elapsedTime - timing::ZOOM_IN_TIME - SLOT_STAGGER * static_cast<float>(slotIndex) };
		if (slotTime < 0.0f)
			return {};

		const float landing{ landingDistance(slotIndex) };

		if (slotTime < RATTLE_END)
		{
			// 震え。左右にも揺らすと「保持できていない」感が出る
			const float scale{ static_cast<float>(m_screen.getHeight()) / static_cast<float>(BASE_SCREEN_HEIGHT) };
			const int amplitude{ std::max(1, static_cast<int>(RATTLE_AMPLITUDE * scale)) };
			std::uniform_int_distribution<int> dist{ -amplitude, amplitude };
			return { dist(m_rng), dist(m_rng) };
		}

		if (slotTime < FALL_END)
		{
			const float t{ (slotTime - RATTLE_END) / FALL_TIME };
			return { 0, static_cast<int>(landing * core::utility::easeIn(t)) };
		}

		const float bounceHeight{ landing * BOUNCE_RATIO };

		if (slotTime < BOUNCE_UP_END)
		{
			const float t{ (slotTime - FALL_END) / BOUNCE_UP_TIME };
			return { 0, static_cast<int>(landing - bounceHeight * core::utility::easeOut(t)) };
		}

		if (slotTime < BOUNCE_DOWN_END)
		{
			const float t{ (slotTime - BOUNCE_UP_END) / BOUNCE_DOWN_TIME };
			return { 0, static_cast<int>(landing - bounceHeight * (1.0f - core::utility::easeIn(t))) };
		}

		// 支えを失って画面外へ抜ける
		const float t{ std::min(1.0f, (slotTime - BOUNCE_DOWN_END) / EXIT_TIME) };
		const float exitDistance{ static_cast<float>(m_screen.getHeight()) };
		return { 0, static_cast<int>(landing + exitDistance * core::utility::easeIn(t)) };
	}
} // namespace game::system::visual
