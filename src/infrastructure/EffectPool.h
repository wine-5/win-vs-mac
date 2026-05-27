#pragma once
#include <vector>
#include "core/ecs/ObjectPool.h"
#include "core/utility/Vector3.h"

namespace infrastructure
{
	struct EffectSlot
	{
		int m_playHandle{ -1 }; // Effekseer プレイハンドル( -1は未使用）
	};

	/**
	 * @brief 1種類のエフェクトをObjectPoolで管理するクラス
	 */
	class EffectPool
	{
	public:
		/**
		 * @brief プールを初期化する
		 * @param resourceHandle LoadEffekseerEffect で取得したリソースハンドル
		 * @param poolSize 初期プールサイズ
		 * @param yOffset 再生位置の Y 軸オフセット（モデル足元からの高さ）
		 * @param scale エフェクトの再生スケール
		 */
		void initialize(int resourceHandle, int poolSize, float yOffset, float scale);

		/**
		 * @brief エフェクトを指定位置で再生し、プレイハンドルを返す
		 * @param position 再生位置
		 * @return Effekseer プレイハンドル（失敗時は -1）
		 */
		int getEffect(core::Vector3 position);

		/**
		 * @brief 指定ハンドルのエフェクトを停止してスロットを返却する
		 * @param playHandle getEffect で取得したプレイハンドル
		 */
		void returnEffect(int playHandle);

		/**
		 * @brief 再生が終了したスロットを自動返却する
		 */
		void update();

		/**
		 * @brief エフェクトがアクティブかどうか
		 */
		bool isActive(int playHandle) const;

	private:
		int   m_resourceHandle{ -1 };
		float m_yOffset{ 0.0f };
		float m_scale  { 1.0f };
		core::base::ObjectPool<EffectSlot> m_pool{};
		std::vector<EffectSlot*> m_activeSlots{};
	};
}