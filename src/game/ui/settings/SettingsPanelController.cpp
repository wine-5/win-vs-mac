#include "SettingsPanelController.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "game/SettingsManager.h"
#include <algorithm>
#include <cmath>

namespace game::ui::settings
{
	using core::data::AudioSettings;
	using core::data::ControlSettings;

	SettingsPanelController::SettingsPanelController(core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    game::SettingsManager& settingsManager)
	    : m_inputProvider{ inputProvider }
	    , m_settingsManager{ settingsManager }
	    , m_inputMapper{ inputProvider }
	    , m_view{ uiRenderer, screen }
	{
	}

	void SettingsPanelController::open()
	{
		m_page = SettingsPage::Sound;
		m_focusIndex = PAGE_COUNT; // 開いた直後は最初の行を選んでおく
		m_draggingRow = -1;

		// 開くのに使ったキー・クリックが押しっぱなしのまま流れ込まないようにする
		m_inputMapper.reset();
		m_prevMouseLeft = m_inputProvider.isMouseLeftPressed();
	}

	SettingsPanelAction SettingsPanelController::update(float deltaTime)
	{
		m_inputMapper.update(deltaTime);

		if (handleMouse())
		{
			m_settingsManager.save();
			return SettingsPanelAction::Close;
		}

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

		if (m_inputMapper.isTriggered(UiAction::Confirm) && !isNavFocused())
			activateRow(getFocusedRow());

		return SettingsPanelAction::None;
	}

	void SettingsPanelController::draw()
	{
		m_view.draw(core::data::GameSettings{ m_settingsManager.getAudio(), m_settingsManager.getControl() },
		    m_page, m_focusIndex, m_inputMapper.isFocusVisible());
	}

	bool SettingsPanelController::handleMouse()
	{
		int mouseX{}, mouseY{};
		m_inputProvider.getMousePosition(mouseX, mouseY);

		const bool isDown{ m_inputProvider.isMouseLeftPressed() };
		const bool isPressed{ isDown && !m_prevMouseLeft };
		m_prevMouseLeft = isDown;

		// つまみを掴んでいる間は、カーソルが行の外へ出ても離すまで追従させる。
		// 掴み直しを強いると、細かく合わせたいときほど扱いにくくなる
		if (m_draggingRow >= 0)
		{
			if (!isDown)
			{
				m_draggingRow = -1;
				return false;
			}

			applySliderRatio(m_draggingRow, m_view.getSliderRatioAt(m_page, mouseX));
			return false;
		}

		const int hovered{ m_view.getFocusIndexAt(m_page, mouseX, mouseY) };

		// 行の上に乗せたら選択位置を移す。左ナビは押して初めて切り替える
		// （乗せただけでページが変わると、通り道の項目に反応して内容が飛ぶ）
		if (hovered >= PAGE_COUNT)
			m_focusIndex = hovered;

		if (!isPressed)
			return false;

		if (m_view.isOnCloseButton(m_page, mouseX, mouseY))
			return true;

		if (hovered >= 0 && hovered < PAGE_COUNT)
		{
			if (static_cast<int>(m_page) != hovered)
			{
				m_page = static_cast<SettingsPage>(hovered);
				m_focusIndex = PAGE_COUNT;
				playUiSe(core::constant::SeType::UiClick);
			}
			return false;
		}

		if (hovered < PAGE_COUNT)
			return false;

		const int row{ hovered - PAGE_COUNT };
		if (getControlKind(m_page, row) == ControlKind::Slider)
		{
			// つまみの上でなくても、線の上を押した位置へ飛ばす（Windowsのスライダーと同じ）
			if (!m_view.isOnSlider(m_page, row, mouseX, mouseY))
				return false;

			m_draggingRow = row;
			applySliderRatio(row, m_view.getSliderRatioAt(m_page, mouseX));
			return false;
		}

		activateRow(row);
		return false;
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
		const bool didChangePage{ isNavFocused() && m_page != static_cast<SettingsPage>(m_focusIndex) };
		if (isNavFocused())
			m_page = static_cast<SettingsPage>(m_focusIndex);

		// ページが変わったときは、行を1つ移るのとは重さが違うので押した音を鳴らす。
		// マウスで左ナビを押したときと同じ音になり、操作の意味と音が一致する
		playUiSe(didChangePage ? core::constant::SeType::UiClick : core::constant::SeType::UiKeyPress);
	}

	void SettingsPanelController::adjustValue(int direction)
	{
		if (isNavFocused())
			return;

		const int row{ getFocusedRow() };
		if (getControlKind(m_page, row) == ControlKind::Button)
			return;

		const ValueRange range{ getValueRange(row) };
		setRowValue(row, std::clamp(getRowValue(row) + range.m_step * direction, range.m_min, range.m_max));
	}

	void SettingsPanelController::activateRow(int row)
	{
		switch (getControlKind(m_page, row))
		{
		case ControlKind::Toggle:
			setRowValue(row, getRowValue(row) != 0 ? 0 : 1);
			break;

		case ControlKind::Button:
			resetCurrentPage();
			break;

		default:
			break;
		}
	}

	void SettingsPanelController::applySliderRatio(int row, float ratio)
	{
		const ValueRange range{ getValueRange(row) };
		const float raw{ range.m_min + ratio * (range.m_max - range.m_min) };

		// 刻みに合わせて丸める。キーボードで動かしたときと同じ値しか取らないようにして、
		// マウスで触ったときだけ半端な数字になるのを防ぐ
		const int steps{ static_cast<int>(std::lround((raw - range.m_min) / range.m_step)) };
		setRowValue(row, std::clamp(range.m_min + steps * range.m_step, range.m_min, range.m_max));
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

	SettingsPanelController::ValueRange SettingsPanelController::getValueRange(int row) const noexcept
	{
		if (m_page == SettingsPage::Sound)
			return { 0, AudioSettings::MAX_LEVEL, AudioSettings::LEVEL_STEP };

		switch (static_cast<ControlRow>(row))
		{
		case ControlRow::Sensitivity:
			return { ControlSettings::MIN_SENSITIVITY, ControlSettings::MAX_SENSITIVITY, 1 };
		case ControlRow::InvertY:
			return { 0, 1, 1 };
		case ControlRow::Shake:
			return { 0, ControlSettings::MAX_SHAKE, ControlSettings::SHAKE_STEP };
		default:
			return { 0, 1, 1 };
		}
	}

	int SettingsPanelController::getRowValue(int row) const noexcept
	{
		if (m_page == SettingsPage::Sound)
		{
			const AudioSettings& audio{ m_settingsManager.getAudio() };
			switch (static_cast<SoundRow>(row))
			{
			case SoundRow::Master: return audio.m_master;
			case SoundRow::Bgm: return audio.m_bgm;
			case SoundRow::Se: return audio.m_se;
			default: return 0;
			}
		}

		const ControlSettings& control{ m_settingsManager.getControl() };
		switch (static_cast<ControlRow>(row))
		{
		case ControlRow::Sensitivity: return control.m_sensitivity;
		case ControlRow::InvertY: return control.m_invertY ? 1 : 0;
		case ControlRow::Shake: return control.m_screenShake;
		default: return 0;
		}
	}

	void SettingsPanelController::setRowValue(int row, int value)
	{
		if (value == getRowValue(row))
			return;

		if (m_page == SettingsPage::Sound)
		{
			AudioSettings audio{ m_settingsManager.getAudio() };
			switch (static_cast<SoundRow>(row))
			{
			case SoundRow::Master: audio.m_master = value; break;
			case SoundRow::Bgm: audio.m_bgm = value; break;
			case SoundRow::Se: audio.m_se = value; break;
			default: return;
			}
			m_settingsManager.setAudio(audio);
		}
		else
		{
			ControlSettings control{ m_settingsManager.getControl() };
			switch (static_cast<ControlRow>(row))
			{
			case ControlRow::Sensitivity: control.m_sensitivity = value; break;
			case ControlRow::InvertY: control.m_invertY = value != 0; break;
			case ControlRow::Shake: control.m_screenShake = value; break;
			default: return;
			}
			m_settingsManager.setControl(control);
		}

		// 1目盛りごとに鳴らす。短く連続して鳴らす前提の音なので間引かない。
		// 効果音の行では、この音そのものが変更後の音量の試聴になる
		playUiSe(core::constant::SeType::UiSliderTick);
	}

	void SettingsPanelController::playUiSe(core::constant::SeType seType) const
	{
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(seType);
	}
} // namespace game::ui::settings
