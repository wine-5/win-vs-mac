#pragma once
#include "core/ecs/ISystem.h"
#include "core/base/EventBus.h"
#include "core/interface/IScreen.h"
#include <array>
#include <random>
#include <vector>

namespace game::system::visual
{
	/**
	 * @brief 落下させるHUDの区別
	 *
	 * ここに無いHUD（プレイヤーステータス・装備スロット等）は落とさない。
	 * 落とすものを増やすときは、ここへ足して SLOT_LAYOUTS に1行加える
	 */
	enum class HudSlot
	{
		Objective, // 左上の目標
		Status,    // 右上の経過時間・難易度
		MiniMap,   // 右上のミニマップ
		Count
	};

	/** @brief HUDの描画位置のずらし量（px） */
	struct HudOffset
	{
		int m_x{ 0 };
		int m_y{ 0 };
	};

	/**
	 * @brief ボス覚醒時にHUDを震わせて落とす演出を担うSystem
	 *
	 * Macが第2形態へ覚醒すると、画面上部のHUDが震えたあと落下して画面外へ消える。
	 * 一度落ちたら戻らない。オフセットの適用はInGameViewが描画時に行う。
	 */
	class HudDropSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param eventBus MacPhaseTransitionEvent購読用のEventBus
		 * @param screen 画面サイズ取得のインターフェース
		 */
		HudDropSystem(core::base::EventBus& eventBus, core::iface::IScreen& screen);

		/**
		 * @brief 落下のタイムラインを進める
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 指定HUDの描画位置のずらし量を返す
		 * @param slot 対象のHUD
		 * @return ずらし量（演出前・対象外なら0）
		 */
		[[nodiscard]] HudOffset getOffset(HudSlot slot) const;

		/**
		 * @brief 指定HUDが画面外まで落ちきったか
		 * @param slot 対象のHUD
		 * @return 落ちきっていればtrue（描画自体を省いてよい）
		 */
		[[nodiscard]] bool isDropped(HudSlot slot) const;

	  private:
		/**
		 * @brief 1つのHUDのずらし量を経過時間から求める
		 * @param slotIndex 対象のHUDの番号
		 * @return ずらし量
		 */
		[[nodiscard]] HudOffset computeOffset(int slotIndex) const;

		/**
		 * @brief 落下して画面下端に接地するまでの距離を返す
		 * @param slotIndex 対象のHUDの番号
		 * @return 接地までの落下距離（px）
		 */
		[[nodiscard]] float landingDistance(int slotIndex) const;

		core::iface::IScreen& m_screen;

		bool m_isPlaying{ false };
		float m_elapsedTime{ 0.0f }; // 演出開始（覚醒イベント）からの経過時間（秒）

		// 接地音を鳴らし終えたHUDの番号。同じHUDで二度鳴らさないために持つ
		std::array<bool, static_cast<size_t>(HudSlot::Count)> m_hasPlayedLandSe{};
		bool m_hasPlayedRattleSe{ false };

		// mutable: ずらし量を返す getOffset は論理的にconstだが、震えの計算で乱数を回す
		mutable std::mt19937 m_rng{ std::random_device{}() };

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual
