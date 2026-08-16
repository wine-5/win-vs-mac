#include "SettingsPanelController.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "game/SettingsManager.h"
#include <algorithm>

namespace game::ui::settings
{
	using core::data::AudioSettings;
	using core::data::ControlSettings;

	SettingsPanelController::SettingsPanelController(core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    game::SettingsManager& settingsManager)
	    : m_settingsManager{ settingsManager }
	    , m_inputMapper{ inputProvider }
	    , m_view{ uiRenderer, screen }
	{
	}

	void SettingsPanelController::open()
	{
		m_page = SettingsPage::Sound;
		m_focusIndex = PAGE_COUNT; // 開いた直後は最初の行を選んでおく

		// 開くのに使ったキーが押しっぱなしのまま流れ込まないようにする
		m_inputMapper.reset();
	}

	SettingsPanelAction SettingsPanelController::update(float deltaTime)
	{
		m_inputMapper.update(deltaTime);

		if (m_inputMapper.isTriggered(UiAction::Cancel))
		{
			// 閉じるときにまとめて保存する。スライダーを動かすたびに書くと
			// ドラッグ中に毎フレームファイルへ書き込むことになる
			m_settingsManager.save();
			return SettingsPanelAction::Close;
		}

		if (m_inputMapper.isTriggered(UiAction::NavigateUp))
			moveFocus(-1);

		if (m_inputMapper.isTriggered(UiAction::NavigateDown))
			moveFocus(1);

		if (m_inputMapper.isTriggered(UiAction::NavigateLeft))
			adjustValue(-1);

		if (m_inputMapper.isTriggered(UiAction::NavigateRight))
			adjustValue(1);

		if (m_inputMapper.isTriggered(UiAction::Confirm))
			confirm();

		return SettingsPanelAction::None;
	}

	void SettingsPanelController::draw()
	{
		m_view.draw(core::data::GameSettings{ m_settingsManager.getAudio(), m_settingsManager.getControl() },
		    m_page, m_focusIndex, m_inputMapper.isFocusVisible());
	}

	void SettingsPanelController::moveFocus(int delta)
	{
		const int lastIndex{ PAGE_COUNT + getRowCount(m_page) - 1 };
		const int next{ std::clamp(m_focusIndex + delta, 0, lastIndex) };
		if (next == m_focusIndex)
			return;

		m_focusIndex = next;

		// 左ナビへ入ったらページも切り替える。Windows の設定と同じで、
		// 選んでいる項目と右に出ている内容が食い違わないようにする
		if (isNavFocused())
			m_page = static_cast<SettingsPage>(m_focusIndex);

		playUiSe(core::constant::SeType::UiKeyPress);
	}

	void SettingsPanelController::adjustValue(int direction)
	{
		if (isNavFocused())
			return;

		const int row{ getFocusedRow() };

		if (m_page == SettingsPage::Sound)
		{
			AudioSettings audio{ m_settingsManager.getAudio() };
			const int step{ AudioSettings::LEVEL_STEP * direction };

			int* target{ nullptr };
			switch (static_cast<SoundRow>(row))
			{
			case SoundRow::Master: target = &audio.m_master; break;
			case SoundRow::Bgm: target = &audio.m_bgm; break;
			case SoundRow::Se: target = &audio.m_se; break;
			default: return; // リセット行は左右では動かさない
			}

			const int next{ std::clamp(*target + step, 0, AudioSettings::MAX_LEVEL) };
			if (next == *target)
				return;

			*target = next;
			m_settingsManager.setAudio(audio);
			playUiSe(core::constant::SeType::UiKeyPress);
			return;
		}

		ControlSettings control{ m_settingsManager.getControl() };
		switch (static_cast<ControlRow>(row))
		{
		case ControlRow::Sensitivity:
		{
			const int next{ std::clamp(control.m_sensitivity + direction,
				ControlSettings::MIN_SENSITIVITY, ControlSettings::MAX_SENSITIVITY) };
			if (next == control.m_sensitivity)
				return;
			control.m_sensitivity = next;
			break;
		}
		case ControlRow::InvertY:
		{
			// トグルは左でオフ・右でオン。スライダーと同じ指の動きで扱えるようにする
			const bool next{ direction > 0 };
			if (next == control.m_invertY)
				return;
			control.m_invertY = next;
			break;
		}
		case ControlRow::Shake:
		{
			const int next{ std::clamp(control.m_screenShake + ControlSettings::SHAKE_STEP * direction,
				0, ControlSettings::MAX_SHAKE) };
			if (next == control.m_screenShake)
				return;
			control.m_screenShake = next;
			break;
		}
		default: return;
		}

		m_settingsManager.setControl(control);
		playUiSe(core::constant::SeType::UiKeyPress);
	}

	void SettingsPanelController::confirm()
	{
		if (isNavFocused())
			return; // ページは選んだ時点で切り替わっているので何もしない

		const int row{ getFocusedRow() };

		if (m_page == SettingsPage::Sound)
		{
			if (static_cast<SoundRow>(row) != SoundRow::Reset)
				return;

			resetCurrentPage();
			return;
		}

		switch (static_cast<ControlRow>(row))
		{
		case ControlRow::InvertY:
		{
			ControlSettings control{ m_settingsManager.getControl() };
			control.m_invertY = !control.m_invertY;
			m_settingsManager.setControl(control);
			playUiSe(core::constant::SeType::UiClick);
			break;
		}
		case ControlRow::Reset:
			resetCurrentPage();
			break;
		default:
			break;
		}
	}

	void SettingsPanelController::resetCurrentPage()
	{
		// 既定値は構造体の初期値がそのまま正なので、新しく作った値で上書きする
		if (m_page == SettingsPage::Sound)
			m_settingsManager.setAudio(AudioSettings{});
		else
			m_settingsManager.setControl(ControlSettings{});

		playUiSe(core::constant::SeType::UiClick);
	}

	void SettingsPanelController::playUiSe(core::constant::SeType seType) const
	{
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(seType);
	}
} // namespace game::ui::settings
