'use strict';

/**
 * デスクトップのロジック層
 * ウィンドウ状態の管理と C++ ↔ JS メッセージングを担う
 */
const DesktopLogic = (function () {
    const winStates = { file: true, param: true, diff: true, rules: false, settings: false };

    let onWindowChangeCallback = null;
    let onVolumeChangeCallback = null;

    // クイック設定で触れるのはマスター音量だけ。BGM・効果音は設定ウィンドウ側に任せる
    let masterVolume = 100;

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
     * @brief タイトルへ戻ることをゲームへ通知する
     * この画面では Esc のポーズメニューを開けないため、ここが唯一の戻り道になる。
     * 確認ダイアログは C++ 側（Win32）が出す
     */
    function backToTitle() {
        sendToGame({ type: 'backToTitle' });
    }

    /**
     * @brief アプリの終了をゲームへ通知する
     */
    function quitGame() {
        sendToGame({ type: 'quitGame' });
    }

    /**
     * @brief Windows アプリ起動をゲームへ通知する
     * @param {'cmd'|'taskmgr'|'recyclebin'} app 起動するアプリ識別子
     */
    function launchApp(app) {
        sendToGame({ type: 'launchApp', app });
    }

    /**
     * @brief 音量が変わったときのコールバックを登録する
     * @param {function(master: number): void} cb
     */
    function onVolumeChange(cb) {
        onVolumeChangeCallback = cb;
    }

    /**
     * @brief 現在のマスター音量を返す
     * @returns {number} 0〜100
     */
    function getMasterVolume() {
        return masterVolume;
    }

    /**
     * @brief マスター音量を変更してゲームへ通知する
     *
     * 送るのは変えた項目だけで、C++側が現在の設定へ混ぜ込む。
     * ここが設定の全項目を知る必要はない
     * @param {number} value 0〜100
     */
    function setMasterVolume(value) {
        if (masterVolume === value) return;

        masterVolume = value;
        sendToGame({ type: 'settingsChanged', audio: { master: value } });
        if (onVolumeChangeCallback) onVolumeChangeCallback(masterVolume);
    }

    /**
     * @brief C++ からのメッセージを処理する
     * @param {object} data 受信したメッセージオブジェクト
     */
    function handleMessage(data) {
        if (data.type === 'settings') {
            // 設定ウィンドウ側で変えられた場合もここへ届くので、トレイの表示が食い違わない
            if (data.audio && typeof data.audio.master === 'number') {
                masterVolume = data.audio.master;
                if (onVolumeChangeCallback) onVolumeChangeCallback(masterVolume);
            }
            return;
        }

        if (data.type === 'windowStateChanged') {
            winStates[data.window] = data.visible;
            if (onWindowChangeCallback) {
                onWindowChangeCallback(data.window, data.visible);
            }
        } else if (data.type === 'tutorialHighlight') {
            // 初回ガイドの締めで「ルール説明.txt」へ送り出す段。
            // ガイドを閉じるまでアイコンを脈打たせ、次に開く場所を体で覚えてもらう
            document.body.classList.toggle('tutorial-highlight-rules', data.show === true);
        } else if (data.type === 'equipReady') {
            // 3つ選び終えた人が次の一手を探して止まらないよう、
            // 出撃の入口（デスクトップの「ゲーム開始.exe」と右下のボタン）を脈打たせる。
            // 装備を外したら false が届くので、強調は元に戻る
            document.body.classList.toggle('equip-ready', data.ready === true);
        }
    }

    return {
        onWindowChange, toggleWindow, startGame, backToTitle, quitGame, launchApp, handleMessage,
        onVolumeChange, getMasterVolume, setMasterVolume
    };
}());

// HTML の onclick から呼ばれるグローバル関数
function toggleWindow(name) { DesktopLogic.toggleWindow(name); }
function openSettingsWindow() { DesktopLogic.toggleWindow('settings'); }
function startGame()        { DesktopLogic.startGame(); }
function backToTitle()      { DesktopLogic.backToTitle(); }
function quitGame()         { DesktopLogic.quitGame(); }
function launchApp(app)     { DesktopLogic.launchApp(app); }

// messaging.js が呼び出すグローバル関数
function onMessageFromGame(data) { DesktopLogic.handleMessage(data); }
