#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include <chrono>
#include <string>

namespace game::ui::ingame
{
	/**
	 * @brief 近づいた設置物の上に「何ができるか」を出す吹き出しView
	 *
	 * ブロックのテクスチャは110ユニットの立方体に貼られ、実際は数十〜数百ユニット
	 * 離れて見る。その距離では細かい絵も文字も潰れるため、テクスチャだけでは
	 * 「何ができるか」を伝えきれない。
	 *
	 * テクスチャは遠くから「他と違うブロックだ」と気づかせる役、
	 * この吹き出しは近づいたときに「何ができるか」を正確に伝える役、と分担する。
	 *
	 * ワールド座標をスクリーンへ落として描くため、対象を見失わない。
	 */
	class InteractPromptView
	{
	  public:
		/**
		 * @brief InteractPromptViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param renderer ワールド座標からスクリーン座標への変換に使う
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager 対象の座標を読むComponentManagerの参照
		 */
		InteractPromptView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IRenderer& renderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 吹き出しを描画する
		 * @param targetId 対象のEntityID（INVALID_ENTITY_IDなら何も描かない）
		 */
		void draw(core::ecs::EntityId targetId);

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IRenderer& m_renderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;

		// 出現の進行（0.0〜1.0）。ぱっと出ると視界の端で見落とすため、
		// 短い時間で浮かび上がらせて動きで気付かせる
		float m_appearProgress{ 0.0f };
		core::ecs::EntityId m_lastTargetId{ core::ecs::INVALID_ENTITY_ID };
		std::chrono::steady_clock::time_point m_lastFrameTime{};
		bool m_hasLastFrameTime{ false };

		// DxLibの描画はShift_JISを期待するため、生成時に一度だけ変換して持つ
		std::string m_keyText{};
		std::string m_actionText{};
	};
} // namespace game::ui::ingame
