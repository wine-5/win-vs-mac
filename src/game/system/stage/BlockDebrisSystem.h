#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/EntityManager.h"
#include "core/interface/IRenderer.h"

namespace game::system::stage
{
	/**
	 * @brief 壊れたブロックの破片を飛散させるSystem
	 *
	 * BlockDebrisComponent を持つEntity（＝壊れた直後のブロック）の破片を毎フレーム動かす。
	 * 破片は「割ってあるモデル」のフレームなので、フレームごとに座標変換を差し込む。
	 * 破片が何個あってもモデル1体ぶんの描画で済む。
	 *
	 * 飛散が終わったらEntityごと破棄する。壊れたブロックは以後どこからも参照されない
	 * ため、残しておく理由が無い。
	 *
	 * @note フレームへ指定するのはローカル→ワールドの行列で、モデル本体の位置・回転・
	 *       スケールを上書きする。そのため描画側がどこへ描こうとしても破片はここで
	 *       決めた場所に出る
	 */
	class BlockDebrisSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief BlockDebrisSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityManager 飛散し終わったブロックを破棄するためのEntityManager
		 * @param renderer フレームの座標変換に使う描画インターフェース
		 */
		BlockDebrisSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityManager& entityManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 破片を動かし、飛散し終わったブロックを破棄する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityManager& m_entityManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system::stage
