#include "AudioManager.h"
#include "DxLib.h"

namespace infrastructure
{
	AudioManager::~AudioManager()
	{
		stopBgm(false);
	}

	void AudioManager::initialize()
	{
		m_repository.initialize();
	}

	void AudioManager::playBgm(core::constant::BgmType type, bool fade)
	{
		const auto& configs{ m_repository.getAllBgmConfigs() };
		auto it{ configs.find(type) };
		if (it == configs.end()) return;

		const BgmConfig& config{ it->second };
		const bool useFade{ fade && config.m_useFade };

		if (m_currentBgmHandle != -1)
		{
			// 別の BGM が再生中: フェードアウトしてから切り替える
			m_pendingBgmHandle = config.m_handle;
			m_pendingBgmVolume = config.m_volume;

			if (useFade)
			{
				m_fadeState = FadeState::FadeOut;
				return;
			}

			StopSoundMem(m_currentBgmHandle);
		}

		// 即時再生
		m_currentBgmHandle = config.m_handle;
		m_targetBgmVolume  = config.m_volume;

		if (useFade)
		{
			m_currentBgmVolume = 0.0f;
			m_fadeState        = FadeState::FadeIn;
		}
		else
		{
			m_currentBgmVolume = config.m_volume;
			m_fadeState        = FadeState::None;
		}

		applyBgmVolume(m_currentBgmHandle, m_currentBgmVolume);
		PlaySoundMem(m_currentBgmHandle, DX_PLAYTYPE_LOOP);
	}

	void AudioManager::stopBgm(bool fade)
	{
		if (m_currentBgmHandle == -1) return;

		m_pendingBgmHandle = -1;

		if (fade)
		{
			m_fadeState       = FadeState::FadeOut;
			m_targetBgmVolume = 0.0f;
		}
		else
		{
			StopSoundMem(m_currentBgmHandle);
			m_currentBgmHandle = -1;
			m_currentBgmVolume = 0.0f;
			m_fadeState        = FadeState::None;
		}
	}

	void AudioManager::playSe(core::constant::SeType type)
	{
		const auto& configs{ m_repository.getAllSeConfigs() };
		auto it{ configs.find(type) };
		if (it == configs.end()) return;

		const SeConfig& config{ it->second };
		ChangeVolumeSoundMem(static_cast<int>(config.m_volume * 255), config.m_handle);
		PlaySoundMem(config.m_handle, DX_PLAYTYPE_BACK);
	}

	void AudioManager::update()
	{
		if (m_fadeState == FadeState::None) return;

		if (m_fadeState == FadeState::FadeIn)
		{
			m_currentBgmVolume += FADE_SPEED;
			if (m_currentBgmVolume >= m_targetBgmVolume)
			{
				m_currentBgmVolume = m_targetBgmVolume;
				m_fadeState        = FadeState::None;
			}
			applyBgmVolume(m_currentBgmHandle, m_currentBgmVolume);
		}
		else if (m_fadeState == FadeState::FadeOut)
		{
			m_currentBgmVolume -= FADE_SPEED;
			if (m_currentBgmVolume <= 0.0f)
			{
				m_currentBgmVolume = 0.0f;
				applyBgmVolume(m_currentBgmHandle, 0.0f);
				StopSoundMem(m_currentBgmHandle);
				m_currentBgmHandle = -1;
				m_fadeState        = FadeState::None;

				// 予約 BGM があればフェードインで再生
				if (m_pendingBgmHandle != -1)
				{
					m_currentBgmHandle = m_pendingBgmHandle;
					m_targetBgmVolume  = m_pendingBgmVolume;
					m_currentBgmVolume = 0.0f;
					m_pendingBgmHandle = -1;
					m_pendingBgmVolume = 0.0f;
					m_fadeState        = FadeState::FadeIn;
					applyBgmVolume(m_currentBgmHandle, 0.0f);
					PlaySoundMem(m_currentBgmHandle, DX_PLAYTYPE_LOOP);
				}
			}
			else
			{
				applyBgmVolume(m_currentBgmHandle, m_currentBgmVolume);
			}
		}
	}

	void AudioManager::applyBgmVolume(int handle, float normalizedVolume) const
	{
		// DxLib の音量は 0〜255 の整数
		const int dxVolume{ static_cast<int>(normalizedVolume * 255) };
		ChangeVolumeSoundMem(dxVolume, handle);
	}
} // namespace infrastructure
