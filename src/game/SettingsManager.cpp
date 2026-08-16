#include "SettingsManager.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/ISettingsRepository.h"

namespace game
{
	SettingsManager::SettingsManager(core::iface::ISettingsRepository& repository)
	    : m_repository{ repository }
	    , m_settings{ repository.load() }
	{
	}

	void SettingsManager::setAudio(const core::data::AudioSettings& audio)
	{
		m_settings.m_audio = audio;
		m_isDirty = true;

		applyAudio();
	}

	void SettingsManager::setControl(const core::data::ControlSettings& control)
	{
		m_settings.m_control = control;
		m_isDirty = true;
	}

	void SettingsManager::applyAudio() const
	{
		// AudioManager はサービスとして登録されるため、参照ではなくここで引く。
		// コンストラクタで受け取ろうとすると、AudioManager より先に生まれる
		// SettingsManager では受け取れない
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->applyVolumeSettings(m_settings.m_audio);
	}

	void SettingsManager::save()
	{
		if (!m_isDirty)
			return;

		m_repository.save(m_settings);
		m_isDirty = false;
	}
} // namespace game
