#pragma once

namespace game::scene
{
	/**
	 * @brief Sceneの基底純粋仮想クラス
	 */
	class IScene
	{
	public:
		virtual ~IScene() = default;

		/**
		 * @brief シーンの更新処理
		 * @param deltaTime フレーム間の時間差
		 */
		virtual void update(float deltaTime) = 0;

		/**
		 * @brief シーンの描画処理
		 */
		virtual void draw() = 0;
	};
}