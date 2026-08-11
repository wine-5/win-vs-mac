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

	  private:
		void applyBonus(core::data::FileExtensionType type);

		/**
		 * @brief 装備から外した拡張子のボーナスを取り消す
		 *
		 * applyBonus の逆。入れ替えは「外す→挿す」の2手で表すので、
		 * 足した値をそのまま引けるようにしておく
		 * @param type 外す拡張子の種別
		 */
		void removeBonus(core::data::FileExtensionType type);

		/**
		 * @brief 持っている拡張子の位置を2つ入れ替える
		 *
		 * 装備中と未装備をまたぐ場合だけ能力が動く。同じ区分どうしなら
		 * 並び替えただけなので、位置は入れ替えるが能力には触らない
		 * @param fromIndex 掴んだ側の位置
		 * @param toIndex 落とした側の位置
		 */
		void swapEquipped(int fromIndex, int toIndex);

		/**
		 * @brief 枠が増えて装備中へ繰り上がった拡張子の効果を乗せる
		 *
		 * 枠を増やす側（BlockBreakSystem）は個数だけを動かす。
		 * 能力の計算はこのSystemに集約したいので、繰り上がりの反映はここで行う
		 * @param maxEquipped 増えたあとの枠数
		 */
		void promoteToEquipped(int maxEquipped);

		/**
		 * @brief 装備中の拡張子の効果へ倍率を掛ける（隔離フォルダの当たり）
		 *
		 * 既に掛かっているぶんとの差だけを足すので、掛けたあとに拡張子を
		 * 挿し外ししても倍率は保たれる。重ねがけはせず、いま掛かっている倍率と
		 * 同じかそれ以下なら何もしない
		 * @param multiplier 掛けたあとの倍率（1.0が素）
		 */
		void multiplyBonuses(float multiplier);

		/**
		 * @brief いま装備中の拡張子へ掛かっている倍率を返す
		 * @return 倍率（持ち物が無ければ1.0）
		 */
		[[nodiscard]] float bonusMultiplier();

		/**
		 * @brief 拡張子1つぶんの効果を倍率つきで足す
		 * @param type 拡張子の種別
		 * @param scale 掛ける倍率
		 */
		void addBonus(core::data::FileExtensionType type, float scale);

		/**
		 * @brief 拡張子1つぶんの効果を倍率つきで引く
		 * @param type 拡張子の種別
		 * @param scale 掛ける倍率
		 */
		void subtractBonus(core::data::FileExtensionType type, float scale);

		core::ecs::ComponentManager& m_componentManager;
		core::base::EventBus& m_eventBus;
		core::iface::IResourceManager& m_resourceManager;
		core::ecs::EntityId m_playerId;

		// イベント中にECSを触らず、次のupdateでまとめて反映する
		std::vector<core::data::FileExtensionType> m_pending{};

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::combat
