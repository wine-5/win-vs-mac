#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"

namespace game::ui::debug
{
	/**
	 * @brief DEBUG: ゲームプレイのワールド空間デバッグ可視化を担当するView
	 *
	 * 当たり判定（Collider）・攻撃範囲・索敵範囲・弾の攻撃範囲をワイヤーフレームで描画する。
	 * InGameViewの3D描画パイプラインの一部として、モデル描画の後に呼ばれることを想定する。
	 * リリース時はこのクラスごと削除する。
	 */
	class DebugGizmoView
	{
	  public:
		/**
		 * @brief DebugGizmoViewのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 3D描画のインターフェース
		 */
		DebugGizmoView(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 有効なギズモをまとめて描画する
		 */
		void draw();

		/**
		 * @brief デバッグ可視化の全体ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setEnabled(bool enabled);

		/**
		 * @brief 当たり判定（Collider）のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setColliderEnabled(bool enabled);

		/**
		 * @brief 攻撃範囲のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setAttackRangeEnabled(bool enabled);

		/**
		 * @brief 索敵範囲のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setDetectionRangeEnabled(bool enabled);

		/**
		 * @brief 弾（Projectile）の攻撃範囲のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setProjectileRangeEnabled(bool enabled);

	  private:
		/**
		 * @brief 当たり判定（青）を可視化する
		 */
		void drawColliders();

		/**
		 * @brief 攻撃範囲（赤）を可視化する
		 */
		void drawAttackRanges();

		/**
		 * @brief 索敵範囲（黄）を可視化する
		 */
		void drawDetectionRanges();

		/**
		 * @brief 弾（Projectile）の攻撃範囲（赤）を可視化する
		 */
		void drawProjectileRanges();

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;

		bool m_isEnabled{ true };
		bool m_isColliderEnabled{ true };
		bool m_isAttackRangeEnabled{ true };
		bool m_isDetectionRangeEnabled{ true };
		bool m_isProjectileRangeEnabled{ false };
	};
} // namespace game::ui::debug
