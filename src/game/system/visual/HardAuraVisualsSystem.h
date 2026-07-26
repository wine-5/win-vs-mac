#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"

namespace core::iface
{
	class IRenderer;        // 前方宣言
	class IResourceManager; // 前方宣言
} // namespace core::iface

namespace game::system::visual
{
	/**
	 * @brief 難易度Hardの敵に赤いオーラを描画するSystem
	 *
	 * Hardでは敵のパラメータが上がるが、数値はプレイヤーからは見えない。
	 * 「この敵はいつもより強い」ことを戦闘中に一目で伝えるため、
	 * 全ての敵を赤い加算グローで包んで通常時と区別する。
	 *
	 * 難易度がNormalのときは何も描かない（生成自体はされる）。
	 * 描画はInGameViewの3D描画フェーズから呼ばれる
	 */
	class HardAuraVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief HardAuraVisualsSystemのコンストラクタ
		 * @param componentManager 敵の位置・体格の読み出しに使うComponentManagerの参照
		 * @param renderer ビルボード描画に使うインターフェース
		 * @param resourceManager 光の画像の読み込みに使うインターフェース
		 * @param isHard 難易度がHardかどうか（falseなら何も描かない）
		 */
		HardAuraVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer,
		    core::iface::IResourceManager& resourceManager,
		    bool isHard);

		/**
		 * @brief 脈動の時間を進める
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 全ての敵の赤いオーラを描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;

		// 光の画像のハンドル（-1なら未ロード。その場合は何も描かない）
		int m_glowHandle{ -1 };

		// 難易度Hardかどうか。falseならdrawは即座に返る
		bool m_isHard{ false };

		float m_elapsedTime{ 0.0f };
	};
} // namespace game::system::visual
