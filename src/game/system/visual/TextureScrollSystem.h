#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"

namespace game::system::visual
{
	/**
	 * @brief テクスチャのずらし量を時間で進めるSystem
	 *
	 * 壁の模様を流して「情報が流れるサーバー内部」を作るための演出。
	 * 描画先を切り替えず、貼り方（UV）をずらすだけなので、
	 * カメラ設定や画面クリアに影響しない。
	 */
	class TextureScrollSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief TextureScrollSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		TextureScrollSystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief ずらし量を進める
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system::visual
