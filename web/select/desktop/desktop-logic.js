'use strict';

/**
 * デスクトップのロジック層
 * ウィンドウ状態の管理と C++ ↔ JS メッセージングを担う
 */
const DesktopLogic = (function () {
    const winStates = { job: true, file: true, param: true, diff: true };

    let onWindowChangeCallback = null;

    /**
     * @brief ウィンドウ表示状態変化時のコールバックを登録する
     * @param {function(name: string, visible: boolean): void} cb
     */
    function onWindowChange(cb) {
        onWindowChangeCallback = cb;
    }

    /**
     * @brief ウィンドウトグルをゲームへ通知する
     * @param {string} name ウィンドウ識別子
     */
    function toggleWindow(name) {
        sendToGame({ type: 'toggleWindow', window: name });
    }

    /**
     * @brief ゲーム開始をゲームへ通知する
     */
    function startGame() {
        sendToGame({ type: 'startGame' });
    }

    /**
     * @brief C++ からのメッセージを処理する
     * @param {object} data 受信したメッセージオブジェクト
     */
    function handleMessage(data) {
        if (data.type === 'windowStateChanged') {
            winStates[data.window] = data.visible;
            if (onWindowChangeCallback) {
                onWindowChangeCallback(data.window, data.visible);
            }
        }
    }

    return { onWindowChange, toggleWindow, startGame, handleMessage };
}());

// HTML の onclick から呼ばれるグローバル関数
function toggleWindow(name) { DesktopLogic.toggleWindow(name); }
function startGame()        { DesktopLogic.startGame(); }

// messaging.js が呼び出すグローバル関数
function onMessageFromGame(data) { DesktopLogic.handleMessage(data); }
