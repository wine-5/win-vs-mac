#pragma once
#include "core/utility/Vector3.h"
#include "core/interface/IRenderer.h"

namespace infrastructure
{
	/**
	 * @brief 3D描画を担当するクラス
	 */
	class Renderer : public core::iface::IRenderer
	{
	public:
		/**
		 * @brief 3Dモデルを描画する
		 * @param modelHandle モデルハンドル
		 * @param position 位置
		 * @param rotation 回転（ラジアン）
		 * @param scale スケール
		 */
		void drawModel(int modelHandle, const core::Vector3& position, const core::Vector3& rotation, const core::Vector3& scale) override;
		
		/**
		 * @brief デバッグ用にコライダーを可視化する
		 * @param center 中心座標
		 * @param size サイズ
		 * @param color 色（ARGB）
		 */
		void drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color) override;

		/**
		 * @brief デバッグ用に球（範囲）を可視化する
		 * @param center 中心座標
		 * @param radius 半径
		 * @param color 色（ARGB）
		 */
		void drawDebugSphere(const core::Vector3& center, float radius, unsigned int color) override;
	};
}