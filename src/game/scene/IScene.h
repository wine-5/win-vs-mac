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

		/**
		 * @brief ポーズの開始・解除を伝える
		 *
		 * ポーズ中はシーンのupdateが呼ばれないため、Win32のサブウィンドウのように
		 * 自前で表示を制御しているものはここで面倒を見る必要がある。
		 * 何もしなくてよいシーンのために既定実装を置く
		 * @param isPaused ポーズ中ならtrue
		 */
		virtual void onPauseChanged([[maybe_unused]] bool isPaused)
		{
		}
	};
} // namespace game::scene