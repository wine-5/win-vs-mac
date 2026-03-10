#pragma once
#include "core/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 描画の純粋仮想クラス
	 * Game層がInfrastructre層に直接依存しないための抽象化
	 */
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		
		/**
		 * @brief 3Dモデルを描画する
		 * @param modelHandle モデルハンドル
		 * @param position 位置
		 * @param rotation 回転（ラジアン）
		 * @param scale スケール
		 */
		virtual void drawModel(int modelHandle, const core::Vector3& position, const::core::Vector3& rotation, const core::Vector3& scale) = 0;
		
		/**
		 * @brief デバッグ用にコライダーを可視化する
		 * @param center 中心座標
		 * @param size サイズ
		 * @param color 色（ARGB）
		 */
		virtual void drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color) = 0;
	};
}