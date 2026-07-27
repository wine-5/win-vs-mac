#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Vector3.h"
#include "game/utility/MiniMapProjection.h"
#include "HudPanel.h"

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム右上のミニマップを描画するView
	 *
	 * 自分の向きが常に上に来る回転式。周囲の一定範囲だけを切り取って映す。
	 * 床の形は GroundSurfaceComponent の実寸と TransformComponent の回転から矩形として描くので、
	 * ミニマップ専用のデータをステージ側に持たせる必要はない。
	 *
	 * 枠からはみ出す中身は UIRenderer の描画範囲制限（矩形）で切り落とす。
	 * 円形にできないのはこの制限のためで、形は角丸の四角形にしている。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class MiniMapView
	{
	  public:
		/**
		 * @brief MiniMapViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager 床・自機・敵の読み出しに使うComponentManagerの参照
		 */
		MiniMapView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief ミニマップを描画する
		 * @param playerId 中心に据えるプレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief 床をすべて描画する
		 *
		 * 自機の高さから離れた床は薄く、さらに離れたものは描かない。
		 * 縦に深く潜るステージなので、絞らないと上下の階層が重なって読めなくなる
		 * @param centerX マップ中心のスクリーンX座標
		 * @param centerY マップ中心のスクリーンY座標
		 * @param playerPosition 自機のワールド座標
		 * @param yaw 自機の向き（ラジアン）
		 * @param scale ワールド1ユニットあたりのピクセル数
		 * @param radius マップの半径（ピクセル）
		 */
		void drawFloors(int centerX, int centerY, const core::Vector3& playerPosition,
		    float yaw, float scale, int radius);

		/**
		 * @brief 回転した矩形を塗りと輪郭で描く
		 * @param centerX 矩形中心のスクリーンX座標
		 * @param centerY 矩形中心のスクリーンY座標
		 * @param halfWidth 幅の半分（ピクセル）
		 * @param halfDepth 奥行きの半分（ピクセル）
		 * @param angle マップ上での回転角（ラジアン）
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param fillAlpha 塗りの不透明度（0〜255）
		 * @param lineAlpha 輪郭の不透明度（0〜255）
		 */
		void drawRotatedRect(float centerX, float centerY, float halfWidth, float halfDepth,
		    float angle, unsigned int color, int fillAlpha, int lineAlpha);

		/**
		 * @brief 敵とボスをすべて描画する
		 * @param centerX マップ中心のスクリーンX座標
		 * @param centerY マップ中心のスクリーンY座標
		 * @param playerPosition 自機のワールド座標
		 * @param yaw 自機の向き（ラジアン）
		 * @param scale ワールド1ユニットあたりのピクセル数
		 * @param radius マップの半径（ピクセル）
		 */
		void drawEnemies(int centerX, int centerY, const core::Vector3& playerPosition,
		    float yaw, float scale, int radius);

		/**
		 * @brief 敵の点を1つ描く
		 *
		 * 範囲外の敵は消さずに縁へ貼り付け、方向だけでも分かるようにする。
		 * 追われていることに気づけないまま囲まれるのを防ぐため
		 * @param centerX マップ中心のスクリーンX座標
		 * @param centerY マップ中心のスクリーンY座標
		 * @param point マップ中心からのピクセル差分
		 * @param radius マップの半径（ピクセル）
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param markerSize 点の大きさ（ピクセル）
		 * @param isBoss ボスなら菱形、雑魚なら円で描く
		 */
		void drawEnemyMarker(int centerX, int centerY, const utility::MiniMapPoint& point,
		    int radius, unsigned int color, int markerSize, bool isBoss);

		/**
		 * @brief 中心に自機の矢印を描く
		 * @param centerX マップ中心のスクリーンX座標
		 * @param centerY マップ中心のスクリーンY座標
		 */
		void drawPlayerArrow(int centerX, int centerY);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		HudPanel m_panel;
	};
} // namespace game::ui::ingame
