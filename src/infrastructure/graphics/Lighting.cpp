#include "Lighting.h"
#include <DxLib.h>

namespace
{
	/** @brief 0-255の色成分をDxLibのCOLOR_F（0.0-1.0）へ変換する */
	COLOR_F toColorF(int r, int g, int b) noexcept
	{
		constexpr float MAX_COMPONENT{ 255.0f };
		return GetColorF(r / MAX_COMPONENT, g / MAX_COMPONENT, b / MAX_COMPONENT, 1.0f);
	}
} // namespace

namespace infrastructure::graphics
{
	void Lighting::setEnabled(bool enabled) noexcept
	{
		SetUseLighting(enabled ? TRUE : FALSE);
	}

	void Lighting::setAmbient(int r, int g, int b) noexcept
	{
		SetGlobalAmbientLight(toColorF(r, g, b));
	}

	void Lighting::setDirectionalLight(const core::Vector3& direction, int r, int g, int b) noexcept
	{
		ChangeLightTypeDir(VGet(direction.x, direction.y, direction.z));
		SetLightDifColor(toColorF(r, g, b));
	}
} // namespace infrastructure::graphics
