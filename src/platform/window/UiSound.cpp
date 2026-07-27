#include "UiSound.h"
#include "WindowConstants.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/SeType.h"
#include "core/utility/Log.h"
#include "thirdparty/nlohmann/json.hpp"
#include <exception>

namespace platform::window
{
	bool tryPlayUiSound(const std::string& json) noexcept
	{
		try
		{
			const auto j{ nlohmann::json::parse(json) };
			if (j.value(WindowConstants::JSON_KEY_TYPE, std::string{}) != WindowConstants::MESSAGE_TYPE_UI_SOUND)
				return false;

			// SE名の指定が無ければ押下音を鳴らす
			const std::string name{ j.value(WindowConstants::JSON_KEY_SE, std::string{ "UiClick" }) };
			const auto seType{ core::constant::toSeType(name) };
			if (seType == core::constant::SeType::None)
				return true;

			auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
			if (audio)
				audio->playSe(seType);

			return true;
		}
		catch (const std::exception& e)
		{
			core::log::error("platform::window::tryPlayUiSound: 処理に失敗しました: {}", e.what());
			return false;
		}
	}
} // namespace platform::window
