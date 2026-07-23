#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"

namespace game::system::combat
{
	/**
	 * @brief 敵の投擲物をプレイヤーのWindow弾で跳ね返すSystem
	 *
	 * プレイヤーが放ったWindow弾（Tag::Player）と敵の投擲物（Tag::Enemy）が
	 * 接触したら、敵の投擲物を逆方向へ跳ね返す（速度反転＋陣営をPlayerへ変更）。
	 * 跳ね返した弾は以降プレイヤー陣営としてAttackSystemが敵へダメージを与える。
	 * レインボー等モデル付きの弾も同じ構造なので特別扱いなしで対応できる。
	 */
	class ProjectileReflectSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		explicit ProjectileReflectSystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 敵弾とプレイヤーWindow弾の接触を判定し、敵弾を跳ね返す
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system::combat
