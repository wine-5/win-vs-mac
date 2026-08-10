#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system::stage
{
	/**
	 * @brief 拡張子の付け替え端末への接近を判定するSystem
	 *
	 * 範囲内に入った端末へ印を付け、Viewが足元の案内を出せるようにする。
	 * 判定と表示を分けているのは、案内の見た目を変えても判定側を触らずに済むため。
	 *
	 * 一番近い端末だけを「近くにいる」と見なす。複数が近接して置かれたときに
	 * 案内が重なって読めなくなるのを防ぐ
	 */
	class RenameTerminalSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief RenameTerminalSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param playerId プレイヤーのEntityID
		 */
		RenameTerminalSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 端末への接近を判定する
		 * @param deltaTime フレーム間の時間差（未使用）
		 */
		void update(float deltaTime) override;

		/**
		 * @brief いま範囲内にいる端末のEntityIDを取得する
		 * @return 端末のEntityID。範囲内に無ければ INVALID_ENTITY_ID
		 */
		[[nodiscard]] core::ecs::EntityId getNearTerminalId() const noexcept;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;

		// 一番近い端末。案内の表示とF2の受付がこれを見る
		core::ecs::EntityId m_nearTerminalId{ core::ecs::INVALID_ENTITY_ID };
	};
} // namespace game::system::stage
