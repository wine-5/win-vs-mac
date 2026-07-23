#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IInputProvider.h"

namespace game
{
	class GameManager; // DEBUG: デバッグモード参照用の前方宣言（リリース時に削除）
} // namespace game

namespace game::system
{
	/**
	 * @brief キーボード入力をInputComponentに反映するSystem
	 */
	class InputSystem : public core::ecs::ISystem
	{
	public:
		static constexpr float INPUT_NEUTRAL = 0.0f;
		static constexpr float INPUT_POSITIVE = 1.0f;
		static constexpr float INPUT_NEGATIVE = -1.0f;

		/**
		 * @brief InputSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityId 対象EntityのID
		 * @param inputProvider IInputProviderの参照
		 * @param gameManager デバッグモード状態の参照（DEBUG: リリース時に削除）
		 */
		InputSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId entityId,
		    core::iface::IInputProvider& inputProvider,
		    GameManager& gameManager);

		/**
		 * @brief 入力を取得しInputComponentを更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_entityId{};
		core::iface::IInputProvider& m_inputProvider;
		GameManager& m_gameManager; // DEBUG: デバッグモード状態の参照（リリース時に削除）
	};
} // namespace game::system
