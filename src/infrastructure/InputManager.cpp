#include "InputManager.h"
#include <DxLib.h>
#include <unordered_map>

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
	}

	// ========== ゲームパッド入力 ==========

	bool InputManager::isPadButtonDown(core::input::GamePadCode code) const
	{
		int input{};
		input = GetJoypadInputState(DX_INPUT_PAD1);
		switch (code)
		{
		case core::input::GamePadCode::ButtonA: return (input & PAD_INPUT_A) != 0;
		case core::input::GamePadCode::ButtonB: return (input & PAD_INPUT_B) != 0;
		case core::input::GamePadCode::DPadUp: return (input & PAD_INPUT_UP) != 0;
		case core::input::GamePadCode::DPadDown: return (input & PAD_INPUT_DOWN) != 0;
		case core::input::GamePadCode::DPadLeft: return (input & PAD_INPUT_LEFT) != 0;
		case core::input::GamePadCode::DPadRight: return (input & PAD_INPUT_RIGHT) != 0;
		default: return false;
		}
	}

	float InputManager::getPadAxis(core::input::GamePadCode code) const
	{
		int x{}, y{};
		GetJoypadAnalogInput(&x, &y, DX_INPUT_PAD1);

		switch (code)
		{
		case core::input::GamePadCode::LeftStickX: return x / 1000.0f;
		case core::input::GamePadCode::LeftStickY: return y / 1000.0f;
		default: return 0.0f;
		}
	}

	bool InputManager::isPadConnected() const
	{
		return GetJoypadInputState(DX_INPUT_PAD1) != -1;
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