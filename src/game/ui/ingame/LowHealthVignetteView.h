#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <chrono>

namespace core::iface
{
	class IResourceManager; // 前方宣言
} // namespace core::iface

namespace game::ui::ingame
{
	/**
	 * @brief 残りHPが少ないときに画面の外周を赤く染めるViewe
	 *
	 * 中央が透明・外周だけが赤いビネット画像を画面全体へ引き伸ばして重ねる。
	 * 視界の中心を塞がずに危険を伝えられるため、瀕死の表現として使う。
	 *
	 * 点滅はHPバーと同じリズムにする（LowHealthPulse.h に計算を共有している）。
	 * 画面のどこを見ていても、同じ心拍で危険が伝わるようにするため
	 */
	class LowHealthVignetteView
	{
	  public:
		/**
		 * @brief LowHealthVignetteViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager HealthComponentの読み出しに使うComponentManagerの参照
		 * @param resourceManager ビネット画像の読み込みに使うIResourceManager
		 */
		LowHealthVignetteView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    core::iface::IResourceManager& resourceManager);

		/**
		 * @brief ビネットを描画する
		 *
		 * 残りHPが警告域に入っていなければ何も描かない
		 * @param playerId HPの読み出し元となるプレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

	  private:
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;

		int m_imageHandle{ -1 };

		// 点滅の基準時刻。描画経路からしか呼ばれずdeltaTimeを受け取らないため、
		// 経過時間は壁時計から求める
		std::chrono::steady_clock::time_point m_startTime{ std::chrono::steady_clock::now() };
	};
} // namespace game::ui::ingame
