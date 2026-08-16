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

		const resource::repository::BgmConfig& config{ it->second };
		const bool useFade{ fade && config.m_useFade };

		// 同じBGMが既に鳴っている場合は何もしない。
		// これが無いと「フェードアウト→同じ曲をフェードイン」が走り、曲に不自然な切れ目ができる
		if (config.m_handle == m_currentBgmHandle && m_fadeState != FadeState::FadeOut)
			return;

		if (m_currentBgmHandle != -1)
		{
			// 別の BGM が再生中: フェードアウトしてから切り替える
			m_pendingBgmHandle = config.m_handle;
			m_pendingBgmSourceVolume = config.m_volume;

			if (useFade)
			{
				m_fadeState = FadeState::FadeOut;
				return;
			}

			// フェード無しでの切り替えはこの場で鳴らし切るので、予約は残さない
			StopSoundMem(m_currentBgmHandle);
			m_pendingBgmHandle = -1;
			m_pendingBgmSourceVolume = 0.0f;
		}

		// 即時再生
		m_currentBgmHandle = config.m_handle;
		m_currentBgmSourceVolume = config.m_volume;
		m_fadeLevel = useFade ? 0.0f : 1.0f;
		m_fadeState = useFade ? FadeState::FadeIn : FadeState::None;

		applyBgmVolume();
		PlaySoundMem(m_currentBgmHandle, DX_PLAYTYPE_LOOP);
	}

	void AudioManager::stopBgm(bool fade)
	{
		if (m_currentBgmHandle == -1) return;

		m_pendingBgmHandle = -1;

		if (fade)
		{
			m_fadeState = FadeState::FadeOut;
		}
		else
		{
			StopSoundMem(m_currentBgmHandle);
			m_currentBgmHandle = -1;
			m_fadeLevel = 0.0f;
			m_currentBgmSourceVolume = 0.0f;
			m_fadeState = FadeState::None;
		}
	}

	void AudioManager::playSe(core::constant::SeType type)
	{
		const auto& configs{ m_repository.getAllSeConfigs() };
		auto it{ configs.find(type) };
		if (it == configs.end()) return;

		// SE は鳴らす瞬間に音量を決めるため、設定を変えても次の一発から自然に追従する
		const resource::repository::SeConfig& config{ it->second };
		const float volume{ config.m_volume * m_volumeSettings.seGain() };
		ChangeVolumeSoundMem(static_cast<int>(volume * DX_VOLUME_MAX), config.m_handle);
		PlaySoundMem(config.m_handle, DX_PLAYTYPE_BACK);
	}

	void AudioManager::update(float deltaTime)
	{
		if (m_fadeState == FadeState::None) return;

		// 1フレームあたりではなく秒あたりの変化量にして、フレームレートに依存させない。
		// 進み具合（0〜1）を動かすので、音源ごとの音量が小さい曲でもフェードにかかる時間は同じになる
		const float step{ deltaTime / FADE_DURATION };

		if (m_fadeState == FadeState::FadeIn)
		{
			m_fadeLevel += step;
			if (m_fadeLevel >= 1.0f)
			{
				m_fadeLevel = 1.0f;
				m_fadeState = FadeState::None;
			}
			applyBgmVolume();
			return;
		}

		m_fadeLevel -= step;
		if (m_fadeLevel > 0.0f)
		{
			applyBgmVolume();
			return;
		}

		m_fadeLevel = 0.0f;
		applyBgmVolume();
		StopSoundMem(m_currentBgmHandle);
		m_currentBgmHandle = -1;
		m_currentBgmSourceVolume = 0.0f;
		m_fadeState = FadeState::None;

		// 予約 BGM があればフェードインで再生
		if (m_pendingBgmHandle == -1)
			return;

		m_currentBgmHandle = m_pendingBgmHandle;
		m_currentBgmSourceVolume = m_pendingBgmSourceVolume;
		m_pendingBgmHandle = -1;
		m_pendingBgmSourceVolume = 0.0f;
		m_fadeState = FadeState::FadeIn;

		applyBgmVolume();
		PlaySoundMem(m_currentBgmHandle, DX_PLAYTYPE_LOOP);
	}

	void AudioManager::applyVolumeSettings(const core::data::AudioSettings& settings)
	{
		m_volumeSettings = settings;

		// 鳴っている曲へその場で反映する。スライダーを動かしている最中に
		// 音が追従しないと、どこで止めればいいのか判断できない
		applyBgmVolume();
	}

	void AudioManager::applyBgmVolume() const
	{
		if (m_currentBgmHandle == -1)
			return;

		const float volume{ m_fadeLevel * m_currentBgmSourceVolume * m_volumeSettings.bgmGain() };
		ChangeVolumeSoundMem(static_cast<int>(volume * DX_VOLUME_MAX), m_currentBgmHandle);
	}
} // namespace infrastructure
