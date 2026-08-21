#include "InputManager.h"
#include <DxLib.h>
#include <unordered_map>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <cstdlib>

namespace infrastructure
{
	// ========== キーボード入力 ==========
	static const std::unordered_map<core::input::KeyCode, int> KEY_MAP = {
		{ core::input::KeyCode::W, KEY_INPUT_W },
		{ core::input::KeyCode::A, KEY_INPUT_A },
		{ core::input::KeyCode::S, KEY_INPUT_S },
		{ core::input::KeyCode::D, KEY_INPUT_D },
		{ core::input::KeyCode::Space, KEY_INPUT_SPACE },
		{ core::input::KeyCode::Enter, KEY_INPUT_RETURN },
		{ core::input::KeyCode::Escape, KEY_INPUT_ESCAPE },

		{ core::input::KeyCode::Up, KEY_INPUT_UP },
		{ core::input::KeyCode::Down, KEY_INPUT_DOWN },
		{ core::input::KeyCode::Left, KEY_INPUT_LEFT },
		{ core::input::KeyCode::Right, KEY_INPUT_RIGHT },

		{ core::input::KeyCode::Shift, KEY_INPUT_LSHIFT },
		{ core::input::KeyCode::Tab, KEY_INPUT_TAB },
		{ core::input::KeyCode::E, KEY_INPUT_E },
		{ core::input::KeyCode::F2, KEY_INPUT_F2 },
	};

	InputManager::InputManager()
	    : m_previousKeyState{}
	{
	}

	void InputManager::captureFrameInput()
	{
		// 自分のウィンドウが非アクティブな間はキー入力を無視する。
		// これが無いと、起動直後やAlt+Tab等でウィンドウの前面/フォーカスが
		// 遷移している最中に CheckHitKey が誤ってキーが押されたままの状態を
		// 返すことがあり、以後 isKeyPressed のエッジ検出が働かなくなる
		// （Escでポーズが開かない等）。
		//
		// ゲーム本体のHWNDと直接比べないのは、セレクト画面がWin32のサブウィンドウ
		// （デスクトップ・ファイル選択など）を前面に出すため。それらが前面のときも
		// 「自分のアプリが操作されている」状態なので、同一プロセスかどうかで判定する
		DWORD foregroundProcessId{};
		GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcessId);
		const bool focused{ foregroundProcessId == GetCurrentProcessId() };

		// マウス視点操作（getMouseDelta）も同じ判定を使う。フレーム中に前面が
		// 入れ替わってもキーとマウスで食い違わないよう、ここで確定させて共有する
		m_isWindowFocused = focused;

		// KEY_MAP内の全キーについて、このフレームで使う状態を一括でスナップショットする。
		// isKeyDownは以後この値を返すだけになるため、フレーム内のどこで何度チェックしても
		// 一貫した値になる（Application/各Scene/各Systemでチェックタイミングがバラバラでも、
		// isKeyPressedのエッジ検出が1フレームずれて取りこぼされることがなくなる）。
		for (const auto& [keyCode, dxKey] : KEY_MAP)
			m_currentKeyState[keyCode] = focused && (CheckHitKey(dxKey) != 0);

		// パッドもここで確定させる。キーボードと同じ理由（フレーム内のどこで読んでも同じ値）
		capturePadInput(focused);

		// どちらの機器で操作しているかも、確定した状態から毎フレーム見る
		if (focused)
			updateLastInputDevice();
	}

	void InputManager::updateLastInputDevice()
	{
		// パッドは、ボタンが押されているかスティックが遊びを抜けていれば触られている。
		// 遊びの処理は storeStick で済んでいるので、0 でなければ倒していると見てよい
		bool isPadActive{ false };
		for (int i{ 0 }; i < PAD_CODE_COUNT && !isPadActive; ++i)
			isPadActive = m_currentPadButtons[i] || m_padAxes[i] != 0.0f;

		if (isPadActive)
		{
			m_lastInputDevice = core::input::InputDevice::GamePad;
			return;
		}

		bool isKeyboardActive{ false };
		for (const auto& [keyCode, isDown] : m_currentKeyState)
		{
			if (!isDown)
				continue;

			isKeyboardActive = true;
			break;
		}

		int mouseX{}, mouseY{};
		GetMousePoint(&mouseX, &mouseY);

		// 初回は基準を取るだけ。前回座標が無いまま差分を取ると必ず動いた扱いになる
		bool isMouseMoved{ false };
		if (m_hasDeviceMousePosition)
		{
			isMouseMoved = std::abs(mouseX - m_deviceMouseX) >= MOUSE_MOVE_THRESHOLD ||
			               std::abs(mouseY - m_deviceMouseY) >= MOUSE_MOVE_THRESHOLD;
		}
		m_deviceMouseX = mouseX;
		m_deviceMouseY = mouseY;
		m_hasDeviceMousePosition = true;

		if (isKeyboardActive || isMouseMoved || isMouseLeftPressed() || isMouseRightPressed())
			m_lastInputDevice = core::input::InputDevice::KeyboardMouse;
	}

	bool InputManager::isKeyDown(core::input::KeyCode keyCode) const
	{
		auto it{ m_currentKeyState.find(keyCode) };
		return it != m_currentKeyState.end() && it->second;
	}

	bool InputManager::isKeyPressed(core::input::KeyCode keyCode) const
	{
		bool currentState{ isKeyDown(keyCode) };
		bool previousState{ m_previousKeyState[keyCode] }; // デフォルトはfalse
		return currentState && !previousState;
	}

	bool InputManager::consumeKeyPress(core::input::KeyCode keyCode)
	{
		if (!isKeyPressed(keyCode))
			return false;

		// 同じフレーム内で2度目以降は成立させない。updateが複数回回っても
		// 「1押し＝1回」を保つ
		return m_consumedKeys.insert(keyCode).second;
	}

	void InputManager::updatePreviousState()
	{
		m_consumedKeys.clear();

		// captureFrameInput()でキャプチャした今フレームの状態を、次フレームの「前回状態」として保存する
		for (const auto& [keyCode, dxKey] : KEY_MAP)
		{
			m_previousKeyState[keyCode] = isKeyDown(keyCode);
		}

		for (int i{ 0 }; i < PAD_CODE_COUNT; ++i)
		{
			m_previousPadButtons[i] = m_currentPadButtons[i];
			m_consumedPadButtons[i] = false;
		}
	}

	// ========== ゲームパッド入力 ==========

	void InputManager::capturePadInput(bool isFocused)
	{
		// 前面でないときは触らない。他アプリを操作している間の入力を拾うと、
		// 戻ってきた瞬間に押しっぱなし扱いのままエッジ検出が働かなくなる
		if (!isFocused)
		{
			clearPadState();
			m_padKind = PadKind::None;
			return;
		}

		// XInputで見えるパッド（Xbox系）を先に試し、駄目ならDirectInputへ落とす。
		// 素のDualShock 4／DualSenseはXInputでは見えないため、この順で両方を拾う
		if (capturePadFromXInput())
		{
			m_padKind = PadKind::XInput;
			return;
		}

		if (capturePadFromDirectInput())
		{
			m_padKind = PadKind::DirectInput;
			return;
		}

		clearPadState();
		m_padKind = PadKind::None;
	}

	bool InputManager::capturePadFromXInput()
	{
		XINPUT_STATE state{};
		if (GetJoypadXInputState(DX_INPUT_PAD1, &state) != 0)
			return false;

		using core::input::GamePadCode;
		const auto set{ [this](GamePadCode code, bool isDown)
			{
			    m_currentPadButtons[static_cast<int>(code)] = isDown;
			} };

		set(GamePadCode::ButtonCross, state.Buttons[XINPUT_BUTTON_A] != 0);
		set(GamePadCode::ButtonCircle, state.Buttons[XINPUT_BUTTON_B] != 0);
		set(GamePadCode::ButtonSquare, state.Buttons[XINPUT_BUTTON_X] != 0);
		set(GamePadCode::ButtonTriangle, state.Buttons[XINPUT_BUTTON_Y] != 0);
		set(GamePadCode::ButtonL1, state.Buttons[XINPUT_BUTTON_LEFT_SHOULDER] != 0);
		set(GamePadCode::ButtonR1, state.Buttons[XINPUT_BUTTON_RIGHT_SHOULDER] != 0);
		set(GamePadCode::ButtonL3, state.Buttons[XINPUT_BUTTON_LEFT_THUMB] != 0);
		set(GamePadCode::ButtonR3, state.Buttons[XINPUT_BUTTON_RIGHT_THUMB] != 0);
		set(GamePadCode::ButtonOptions, state.Buttons[XINPUT_BUTTON_START] != 0);
		set(GamePadCode::ButtonShare, state.Buttons[XINPUT_BUTTON_BACK] != 0);
		set(GamePadCode::DPadUp, state.Buttons[XINPUT_BUTTON_DPAD_UP] != 0);
		set(GamePadCode::DPadDown, state.Buttons[XINPUT_BUTTON_DPAD_DOWN] != 0);
		set(GamePadCode::DPadLeft, state.Buttons[XINPUT_BUTTON_DPAD_LEFT] != 0);
		set(GamePadCode::DPadRight, state.Buttons[XINPUT_BUTTON_DPAD_RIGHT] != 0);

		// トリガーは0〜255。踏み込み量をそのまま持ちつつ、押し切りをボタンとしても拾えるようにする
		const float leftTrigger{ state.LeftTrigger / 255.0f };
		const float rightTrigger{ state.RightTrigger / 255.0f };
		m_padAxes[static_cast<int>(GamePadCode::AxisL2)] = leftTrigger;
		m_padAxes[static_cast<int>(GamePadCode::AxisR2)] = rightTrigger;
		set(GamePadCode::ButtonL2, leftTrigger >= TRIGGER_THRESHOLD);
		set(GamePadCode::ButtonR2, rightTrigger >= TRIGGER_THRESHOLD);

		// XInputのY軸は上が正。ゲーム側も上（前方）を正で扱うのでそのまま渡す
		constexpr float STICK_RANGE{ 32767.0f };
		storeStick(GamePadCode::LeftStickX, GamePadCode::LeftStickY,
		    state.ThumbLX / STICK_RANGE, state.ThumbLY / STICK_RANGE);
		storeStick(GamePadCode::RightStickX, GamePadCode::RightStickY,
		    state.ThumbRX / STICK_RANGE, state.ThumbRY / STICK_RANGE);

		return true;
	}

	bool InputManager::capturePadFromDirectInput()
	{
		DINPUT_JOYSTATE state{};
		if (GetJoypadDirectInputState(DX_INPUT_PAD1, &state) != 0)
			return false;

		using core::input::GamePadCode;

		// DualShock 4／DualSenseをWindows標準のドライバで挿したときのボタンの並び。
		// 機種によって並びは変わるが、開発機がDualShock 4なのでこれを既定にする
		constexpr int DI_SQUARE{ 0 };
		constexpr int DI_CROSS{ 1 };
		constexpr int DI_CIRCLE{ 2 };
		constexpr int DI_TRIANGLE{ 3 };
		constexpr int DI_L1{ 4 };
		constexpr int DI_R1{ 5 };
		constexpr int DI_L2{ 6 };
		constexpr int DI_R2{ 7 };
		constexpr int DI_SHARE{ 8 };
		constexpr int DI_OPTIONS{ 9 };
		constexpr int DI_L3{ 10 };
		constexpr int DI_R3{ 11 };

		const auto isDown{ [&state](int index)
			{ return state.Buttons[index] != 0; } };
		const auto set{ [this](GamePadCode code, bool down)
			{
			    m_currentPadButtons[static_cast<int>(code)] = down;
			} };

		set(GamePadCode::ButtonSquare, isDown(DI_SQUARE));
		set(GamePadCode::ButtonCross, isDown(DI_CROSS));
		set(GamePadCode::ButtonCircle, isDown(DI_CIRCLE));
		set(GamePadCode::ButtonTriangle, isDown(DI_TRIANGLE));
		set(GamePadCode::ButtonL1, isDown(DI_L1));
		set(GamePadCode::ButtonR1, isDown(DI_R1));
		set(GamePadCode::ButtonL2, isDown(DI_L2));
		set(GamePadCode::ButtonR2, isDown(DI_R2));
		set(GamePadCode::ButtonShare, isDown(DI_SHARE));
		set(GamePadCode::ButtonOptions, isDown(DI_OPTIONS));
		set(GamePadCode::ButtonL3, isDown(DI_L3));
		set(GamePadCode::ButtonR3, isDown(DI_R3));

		// 十字キーはボタンではなくハットスイッチ。角度（1/100度）で来るので方向へ展開する。
		// 斜めは隣り合う2方向がどちらも押されている扱いになる
		constexpr unsigned int POV_NONE{ 0xffffffffu };
		constexpr unsigned int POV_UP{ 0 };
		constexpr unsigned int POV_UP_RIGHT{ 4500 };
		constexpr unsigned int POV_RIGHT{ 9000 };
		constexpr unsigned int POV_DOWN_RIGHT{ 13500 };
		constexpr unsigned int POV_DOWN{ 18000 };
		constexpr unsigned int POV_DOWN_LEFT{ 22500 };
		constexpr unsigned int POV_LEFT{ 27000 };
		constexpr unsigned int POV_UP_LEFT{ 31500 };

		const unsigned int pov{ state.POV[0] };
		const bool hasPov{ pov != POV_NONE };
		set(GamePadCode::DPadUp, hasPov && (pov == POV_UP_LEFT || pov == POV_UP || pov == POV_UP_RIGHT));
		set(GamePadCode::DPadRight, hasPov && (pov == POV_UP_RIGHT || pov == POV_RIGHT || pov == POV_DOWN_RIGHT));
		set(GamePadCode::DPadDown, hasPov && (pov == POV_DOWN_RIGHT || pov == POV_DOWN || pov == POV_DOWN_LEFT));
		set(GamePadCode::DPadLeft, hasPov && (pov == POV_DOWN_LEFT || pov == POV_LEFT || pov == POV_UP_LEFT));

		// DirectInputのY軸は下が正。ゲーム側は上を正で扱うので符号を反転させる
		constexpr float AXIS_RANGE{ 1000.0f };
		storeStick(GamePadCode::LeftStickX, GamePadCode::LeftStickY,
		    state.X / AXIS_RANGE, -state.Y / AXIS_RANGE);
		storeStick(GamePadCode::RightStickX, GamePadCode::RightStickY,
		    state.Z / AXIS_RANGE, -state.Rz / AXIS_RANGE);

		// L2/R2の踏み込み量。休めた状態が-1000、踏み切りが1000で来るので0〜1へ直す。
		// 押し切りの判定はデジタルのボタン側（DI_L2/DI_R2）を使うので、こちらは目安
		m_padAxes[static_cast<int>(GamePadCode::AxisL2)] = (state.Rx / AXIS_RANGE + 1.0f) * 0.5f;
		m_padAxes[static_cast<int>(GamePadCode::AxisR2)] = (state.Ry / AXIS_RANGE + 1.0f) * 0.5f;

		return true;
	}

	void InputManager::storeStick(core::input::GamePadCode xCode, core::input::GamePadCode yCode,
	    float rawX, float rawY)
	{
		const float magnitude{ std::sqrt(rawX * rawX + rawY * rawY) };

		if (magnitude <= STICK_DEADZONE)
		{
			m_padAxes[static_cast<int>(xCode)] = 0.0f;
			m_padAxes[static_cast<int>(yCode)] = 0.0f;
			return;
		}

		// 遊びを抜いたぶんを0〜1へ引き伸ばす。こうしないと、遊びを抜けた瞬間に
		// いきなりSTICK_DEADZONE分の速さで動き出してしまう
		const float clamped{ magnitude > 1.0f ? 1.0f : magnitude };
		const float scale{ (clamped - STICK_DEADZONE) / (1.0f - STICK_DEADZONE) / magnitude };

		m_padAxes[static_cast<int>(xCode)] = rawX * scale;
		m_padAxes[static_cast<int>(yCode)] = rawY * scale;
	}

	void InputManager::clearPadState() noexcept
	{
		for (int i{ 0 }; i < PAD_CODE_COUNT; ++i)
		{
			m_currentPadButtons[i] = false;
			m_padAxes[i] = 0.0f;
		}
	}

	bool InputManager::isPadButtonDown(core::input::GamePadCode code) const
	{
		return m_currentPadButtons[static_cast<int>(code)];
	}

	bool InputManager::isPadButtonPressed(core::input::GamePadCode code) const
	{
		const int index{ static_cast<int>(code) };
		return m_currentPadButtons[index] && !m_previousPadButtons[index];
	}

	bool InputManager::consumePadPress(core::input::GamePadCode code)
	{
		if (!isPadButtonPressed(code))
			return false;

		// 同じフレーム内で2度目以降は成立させない（consumeKeyPressと同じ）
		const int index{ static_cast<int>(code) };
		if (m_consumedPadButtons[index])
			return false;

		m_consumedPadButtons[index] = true;
		return true;
	}

	float InputManager::getPadAxis(core::input::GamePadCode code) const
	{
		return m_padAxes[static_cast<int>(code)];
	}

	bool InputManager::isPadConnected() const
	{
		return m_padKind != PadKind::None;
	}

	core::input::InputDevice InputManager::getLastInputDevice() const
	{
		return m_lastInputDevice;
	}

	// ========== マウス入力 ==========

	void InputManager::getMousePosition(int& outX, int& outY) const
	{
		GetMousePoint(&outX, &outY);
	}

	bool InputManager::isMouseLeftPressed() const
	{
		return (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
	}

	bool InputManager::isMouseRightPressed() const
	{
		return (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;
	}

	void InputManager::getMouseDelta(int& outDx, int& outDy)
	{
		outDx = 0;
		outDy = 0;

		// 他アプリが前面の間は視点を動かさず、カーソルにも触れない。
		// ここで抜けないと、中央へ戻す SetMousePoint が他アプリを操作中の
		// カーソルを毎フレーム引き戻して中央に貼り付いたように見え、さらに
		// ゲーム画面の外にあるカーソル座標がそのまま移動量になってカメラが回り続ける
		if (!m_isWindowFocused)
		{
			m_hasPreviousMousePosition = false; // 戻ってきたら現在位置を基準に取り直す
			m_needsMouseRecenter = true;
			return;
		}

		int mouseX{}, mouseY{};
		GetMousePoint(&mouseX, &mouseY);

		// カーソル表示中は現在座標と前回座標の差分を使う（カーソルは自由移動）
		if (m_cursorVisible)
		{
			if (!m_hasPreviousMousePosition)
			{
				m_previousMouseX = mouseX;
				m_previousMouseY = mouseY;
				m_hasPreviousMousePosition = true;
				outDx = 0;
				outDy = 0;
				return;
			}

			outDx = mouseX - m_previousMouseX;
			outDy = mouseY - m_previousMouseY;
			m_previousMouseX = mouseX;
			m_previousMouseY = mouseY;
			return;
		}

		int screenWidth{}, screenHeight{};
		GetDrawScreenSize(&screenWidth, &screenHeight);
		const int centerX{ screenWidth / 2 };
		const int centerY{ screenHeight / 2 };

		// 前面へ戻った直後は中央へ置き直すだけにする。他アプリを操作していた間に
		// カーソルは中央から離れているので、そのぶんを差分にすると視点が飛ぶ
		if (m_needsMouseRecenter)
		{
			m_needsMouseRecenter = false;
			SetMousePoint(centerX, centerY);
			return;
		}

		outDx = mouseX - centerX;
		outDy = mouseY - centerY;

		// 次フレームのため中央に戻す（画面端で止まらず無限にマウスを動かせるようにする)
		SetMousePoint(centerX, centerY);
	}

	void InputManager::movePointer(int deltaX, int deltaY)
	{
		// 前面でないときは触らない。他のアプリを操作している最中にカーソルが
		// 動くと、ゲームの外へ操作が漏れる
		if (!m_isWindowFocused)
			return;

		if (deltaX == 0 && deltaY == 0)
			return;

		POINT cursor{};
		if (GetCursorPos(&cursor) == 0)
			return;

		int x{ cursor.x + deltaX };
		int y{ cursor.y + deltaY };

		// ゲームのウィンドウの中へ丸める。別のモニタや他のアプリの上へは出さない
		RECT windowRect{};
		if (auto* hwnd{ static_cast<HWND>(GetMainWindowHandle()) };
		    hwnd != nullptr && GetWindowRect(hwnd, &windowRect) != 0)
		{
			x = std::clamp(x, static_cast<int>(windowRect.left),
			    static_cast<int>(windowRect.right) - 1);
			y = std::clamp(y, static_cast<int>(windowRect.top),
			    static_cast<int>(windowRect.bottom) - 1);
		}

		SetCursorPos(x, y);
	}

	void InputManager::clickPointer()
	{
		// 前面でないときは送らない。他のアプリを勝手に操作してしまう
		if (!m_isWindowFocused)
			return;

		// 押して離すまでを1回で送る。素早く2回呼ばれれば、OSが時間差を見て
		// ダブルクリックとして扱う（デスクトップのアイコンはこれで開ける）
		INPUT inputs[2]{};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

		SendInput(static_cast<UINT>(std::size(inputs)), inputs, sizeof(INPUT));
	}

	void InputManager::setMouseCursorVisible(bool visible)
	{
		if (m_cursorVisible == visible)
			return;

		m_cursorVisible = visible;
		SetMouseDispFlag(visible ? TRUE : FALSE);

		// 非表示にする瞬間は同時にカーソルを中央へ置き、初回の移動量が大きく飛ぶのを防ぐ
		if (!visible)
		{
			int screenWidth{}, screenHeight{};
			GetDrawScreenSize(&screenWidth, &screenHeight);
			const int centerX{ screenWidth / 2 };
			const int centerY{ screenHeight / 2 };
			SetMousePoint(centerX, centerY);
			m_previousMouseX = centerX;
			m_previousMouseY = centerY;
			m_hasPreviousMousePosition = true;
		}
		else
		{
			// 表示に戻すときは現在位置を基準に次回差分を計算する
			GetMousePoint(&m_previousMouseX, &m_previousMouseY);
			m_hasPreviousMousePosition = true;
		}
	}
} // namespace infrastructure