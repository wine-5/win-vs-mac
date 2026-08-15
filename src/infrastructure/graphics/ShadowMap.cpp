#include "ShadowMap.h"
#include <DxLib.h>
#include "core/utility/Log.h"

namespace
{
	/// @brief 使用するシャドウマップのスロット番号（DxLibは0〜2の3枚まで同時に使える）
	constexpr int SLOT_INDEX{ 0 };
} // namespace

namespace infrastructure::graphics
{
	ShadowMap::~ShadowMap()
	{
		destroy();
	}

	bool ShadowMap::create(int resolution)
	{
		destroy();

		m_handle = MakeShadowMap(resolution, resolution);
		if (m_handle == -1)
		{
			core::log::error("シャドウマップの作成に失敗しました: resolution={}", resolution);
			return false;
		}
		return true;
	}

	void ShadowMap::destroy() noexcept
	{
		if (m_handle == -1)
			return;

		DeleteShadowMap(m_handle);
		m_handle = -1;
	}

	bool ShadowMap::isValid() const noexcept
	{
		return m_handle != -1;
	}

	void ShadowMap::setLightDirection(const core::Vector3& direction) noexcept
	{
		if (m_handle == -1)
			return;
		SetShadowMapLightDirection(m_handle, VGet(direction.x, direction.y, direction.z));
	}

	void ShadowMap::setDrawArea(const core::Vector3& center, float halfSize, float halfHeight) noexcept
	{
		if (m_handle == -1)
			return;

		SetShadowMapDrawArea(m_handle,
		    VGet(center.x - halfSize, center.y - halfHeight, center.z - halfSize),
		    VGet(center.x + halfSize, center.y + halfHeight, center.z + halfSize));
	}

	void ShadowMap::setAdjustDepth(float depth) noexcept
	{
		if (m_handle == -1)
			return;
		SetShadowMapAdjustDepth(m_handle, depth);
	}

	void ShadowMap::beginCasterPass() noexcept
	{
		if (m_handle == -1)
			return;
		ShadowMap_DrawSetup(m_handle);
	}

	void ShadowMap::endCasterPass() noexcept
	{
		if (m_handle == -1)
			return;
		ShadowMap_DrawEnd();
	}

	void ShadowMap::beginReceivePass() noexcept
	{
		if (m_handle == -1)
			return;
		SetUseShadowMap(SLOT_INDEX, m_handle);
	}

	void ShadowMap::endReceivePass() noexcept
	{
		if (m_handle == -1)
			return;

		// -1を渡すとスロットの割り当てが外れる。外し忘れると以降の描画（UI含む）にも
		// 影の判定が乗り続けるため、受ける物を描き終えたら必ず戻す
		SetUseShadowMap(SLOT_INDEX, -1);
	}
} // namespace infrastructure::graphics
