#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"
#include "core/data/FileExtensionType.h"
#include "core/interface/IResourceManager.h"
#include <vector>

namespace game::system::combat
{
	/**
	 * @brief 拾った拡張子をプレイヤーの能力へ反映するSystem
	 *
	 * ExtensionPickedUpEvent を購読し、extensionBonus.json の値を
	 * 生きているコンポーネント（AttackComponent・HealthComponent・
	 * PlayerStatsComponent）へ加算する。
	 *
	 * セレクト画面で選んだファイルはInGame生成時に PlayerData へ反映されるが、
	 * こちらは走っている最中に増えるため、コンポーネントを直接触る必要がある。
	 *
	 * @note 現状は拾った瞬間に効果が乗る暫定実装。本来はインベントリへ入れて、
	 *       リネームブロックの前でスロットへ挿し替える設計（extension_swap_design.md）。
	 *       無制限に積めるとバランスが壊れるため、InGameで乗る個数だけ制限してある
	 */
	class ExtensionEquipSystem : public core::ecs::ISystem
	{
	  public:
		/// @brief InGame中に能力へ乗せられる拡張子の個数
		///
		/// セレクト画面の3つと合わせて最大6つになる（設計上の取り決め）
		static constexpr int MAX_INGAME_SLOTS{ 3 };

		/**
		 * @brief ExtensionEquipSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus ExtensionPickedUpEvent購読用のEventBus
		 * @param resourceManager 拡張子ボーナスを引くリソース管理インターフェース
		 * @param playerId プレイヤーのEntityID
		 */
		ExtensionEquipSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::iface::IResourceManager& resourceManager,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 拾った拡張子の効果を反映する
		 * @param deltaTime フレーム間の時間差（未使用）
		 */
		void update(float deltaTime) override;

		/**
		 * @brief InGameで拾って効果が乗っている拡張子の数を取得する
		 * @return 反映済みの個数
		 */
		[[nodiscard]] int getEquippedCount() const noexcept;

	  private:
		void applyBonus(core::data::FileExtensionType type);

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;
		core::ecs::EntityId m_playerId;

		// イベント中にECSを触らず、次のupdateでまとめて反映する
		std::vector<core::data::FileExtensionType> m_pending{};

		// 効果を反映済みの個数（MAX_INGAME_SLOTSで頭打ち）
		int m_equippedCount{ 0 };

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::combat
