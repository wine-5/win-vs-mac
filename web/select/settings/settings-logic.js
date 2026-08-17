'use strict';

/**
 * 設定の値の保持と、C++とのやり取りを受け持つ。
 *
 * 値の正はC++側（SettingsManager）にあり、ここはその写しを持って表示するだけ。
 * 変更のたびに送り返し、保存もC++が行う
 */
const SettingsLogic = (function () {
    // 既定値。C++の core::data::GameSettings の初期値と一致させること
    const DEFAULTS = {
        audio: { master: 100, bgm: 100, se: 100 },
        control: { sensitivity: 5, invertY: false, screenShake: 100 }
    };

    // どのキーがどちらの区分に属するか。行を増やすときはここにも足す
    const AUDIO_KEYS = ['master', 'bgm', 'se'];
    const CONTROL_KEYS = ['sensitivity', 'invertY', 'screenShake'];

    let settings = clone(DEFAULTS);
    let onChangeCallback = null;

    function clone(source) {
        return JSON.parse(JSON.stringify(source));
    }

    /**
     * キーが属する区分（'audio' か 'control'）を返す
     * @param {string} key 設定のキー
     * @returns {string} 区分名
     */
    function groupOf(key) {
        return AUDIO_KEYS.indexOf(key) >= 0 ? 'audio' : 'control';
    }

    /**
     * 現在の設定を返す
     * @returns {object} 設定のコピー
     */
    function getSettings() {
        return clone(settings);
    }

    /**
     * 1つの値を変更してC++へ送る
     * @param {string} key 設定のキー
     * @param {number|boolean} value 新しい値
     */
    function setValue(key, value) {
        const group = groupOf(key);
        if (settings[group][key] === value) return;

        settings[group][key] = value;
        notify();
    }

    /**
     * ページ単位で既定値へ戻す
     * @param {string} page 'sound' か 'control'
     */
    function resetPage(page) {
        const group = page === 'sound' ? 'audio' : 'control';
        settings[group] = clone(DEFAULTS[group]);
        notify();
    }

    /**
     * C++から届いた設定で丸ごと置き換える（画面を開いた直後の初期値）
     * @param {object} data 受け取ったメッセージ
     */
    function onMessageFromGame(data) {
        applyDifficultyTheme(data);

        if (!data || data.type !== 'settings') return;

        if (data.audio) settings.audio = Object.assign(clone(DEFAULTS.audio), data.audio);
        if (data.control) settings.control = Object.assign(clone(DEFAULTS.control), data.control);

        // 受け取った値はC++が持っているものなので、そのまま送り返さず表示だけ更新する
        if (onChangeCallback) onChangeCallback(getSettings());
    }

    function notify() {
        sendToGame({ type: 'settingsChanged', audio: settings.audio, control: settings.control });
        if (onChangeCallback) onChangeCallback(getSettings());
    }

    /**
     * 値が変わったときに呼ぶ処理を登録する
     * @param {Function} callback 表示を更新する処理
     */
    function onChange(callback) {
        onChangeCallback = callback;
    }

    return {
        getSettings: getSettings,
        setValue: setValue,
        resetPage: resetPage,
        onChange: onChange,
        onMessageFromGame: onMessageFromGame
    };
}());

// messaging.js から呼ばれる受け口
function onMessageFromGame(data) {
    SettingsLogic.onMessageFromGame(data);
}
