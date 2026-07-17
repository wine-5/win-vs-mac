#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
	/**
	 * @brief カメラ前方（レティクル中心）の敵捕捉を判定するSystem
	 *
	 * AimComponent＋CameraComponentを持つ全エンティティ（＝カメラで照準する主体）を走査し、
	 * m_forwardと別陣営の対象の方向のなす角から捕捉を判定してAimComponentへ書き込む。
	 * 特定のIDに依存しないため、将来カメラを持つ主体が増えても自動的に対応できる。
	 */
	class TargetingSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief TargetingSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		explicit TargetingSystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 各照準主体の敵捕捉を判定してAimComponentを更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system
