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

	int Lighting::createPointLight(const core::Vector3& position, float range, int r, int g, int b)
	{
		// 減衰は 1/(a0 + a1*d + a2*d^2)。rangeの端で十分暗くなるよう線形項で調整する
		constexpr float ATTEN_CONST{ 1.0f };
		const float attenLinear{ (range > 0.0f) ? (2.0f / range) : 0.0f };
		constexpr float ATTEN_QUAD{ 0.0f };

		const int handle{ CreatePointLightHandle(
			VGet(position.x, position.y, position.z), range, ATTEN_CONST, attenLinear, ATTEN_QUAD) };
		if (handle == -1)
			return -1;

		SetLightDifColorHandle(handle, toColorF(r, g, b));
		SetLightEnableHandle(handle, TRUE);
		return handle;
	}

	void Lighting::setPointLightPosition(int lightHandle, const core::Vector3& position) noexcept
	{
		if (lightHandle == -1)
			return;
		SetLightPositionHandle(lightHandle, VGet(position.x, position.y, position.z));
	}

	void Lighting::destroyPointLight(int lightHandle) noexcept
	{
		if (lightHandle == -1)
			return;
		DeleteLightHandle(lightHandle);
	}
} // namespace infrastructure::graphics
