#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/base/EventBus.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResourceManager.h"
#include "game/event/InGameEvents.h"
#include <random>
#include <vector>

namespace core::iface
{
	class IStringConverter; // 前方宣言
} // namespace core::iface

namespace game::system::visual
{
	/**
	 * @brief 敵の発見演出（頭上のiOS通知バナー風アラート）を担当するSystem
	 *
	 * EnemyAlertedEvent受信時に対象へAlertComponentを付与し、一定時間、敵の頭上に
	 * 「赤い『！』アプリアイコン＋危険メッセージ」の横長バナーを表示する
	 * （出現時に少しせり上がってフェードイン、最後にフェードアウト）。
	 * メッセージは発見時に数種からランダムで選ぶ。
	 * updateでタイマーを進め、drawはInGameViewの描画フェーズから呼ばれる。
	 * 敵の種類に依らずEnemyAlertedEvent経由で動くため、Xcode/Safari/Mac共通で使える。
	 */
	class DetectionAlertVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief DetectionAlertVisualsSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus EnemyAlertedEvent購読用のEventBusの参照
		 * @param renderer ワールド→スクリーン変換に使うIRenderer
		 * @param uiRenderer バナー描画に使うIUIRenderer
		 * @param screen 画面サイズ取得のインターフェース
		 * @param resourceManager アイコン画像の読み込みに使うIResourceManager
		 */
		DetectionAlertVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::iface::IRenderer& renderer,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IResourceManager& resourceManager);

		/**
		 * @brief バナーの表示タイマーを進め、尽きたら消す
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 表示中の敵の頭上に通知バナーを描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		void onEnemyAlerted(const event::EnemyAlertedEvent& e);

		/**
		 * @brief 1体分の通知バナーを画面に描画する
		 * @param centerX バナー中心のスクリーンX
		 * @param topY バナー上端のスクリーンY
		 * @param enemyName ヘッダーに出す敵の種類名（＝アプリ名。"Xcode"等のASCII）
		 * @param messageUtf8 本文に出す危険メッセージ（UTF-8）
		 */
		void drawBanner(int centerX, int topY, const char* enemyName, const char* messageUtf8);

		/**
		 * @brief 角丸の塗り矩形を描画する（DxLibに角丸矩形が無いため矩形＋円で合成）
		 * @param x 左上X
		 * @param y 左上Y
		 * @param width 幅
		 * @param height 高さ
		 * @param radius 角の半径
		 * @param color 色（ARGB）
		 */
		void drawRoundedRect(int x, int y, int width, int height, int radius, unsigned int color);

		core::ecs::ComponentManager& m_componentManager;
		core::base::EventBus& m_eventBus;
		core::iface::IRenderer& m_renderer;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		// DxLibはShift-JISで描画するため、日本語メッセージの変換に使う（無ければ変換せず素通し）
		core::iface::IStringConverter* m_stringConverter{ nullptr };
		// 左のアプリアイコン画像のハンドル（-1なら未ロード。その場合はアイコンを描かない）
		int m_iconHandle{ -1 };
		// 通知バーの背景画像のハンドル（-1なら未ロード。その場合は角丸矩形で代替描画）
		int m_barHandle{ -1 };
		std::mt19937 m_rng{ std::random_device{}() };

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual
