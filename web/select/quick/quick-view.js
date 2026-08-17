'use strict';

/**
 * クイック設定の表示と操作。
 *
 * 値の正はC++（SettingsManager）が持つ。ここは受け取って表示し、
 * 触られたぶんを送り返すだけで、自前では保持しない
 */
(function () {
    const slider = document.getElementById('master');

    /**
     * 音量の表示（スライダー・数値・スピーカーの波）を更新する
     * @param {number} master マスター音量（0〜100）
     */
    function render(master) {
        slider.value = master;
        slider.style.setProperty('--fill', master + '%');
        document.getElementById('value').textContent = master + '%';
        document.getElementById('volume-icon').innerHTML = speakerSvg(master);
    }

    /**
     * 音量に応じた波の数のスピーカーSVGを返す
     * @param {number} master マスター音量（0〜100）
     * @returns {string} SVG文字列
     */
    function speakerSvg(master) {
        let waves;
        if (master <= 0) {
            waves = '<path d="M13 7.5 17 12.5M17 7.5 13 12.5" stroke-linecap="round"/>';
        } else if (master < 50) {
            waves = '<path d="M13 7.5a3.5 3.5 0 0 1 0 5" stroke-linecap="round"/>';
        } else {
            waves = '<path d="M13 7.5a3.5 3.5 0 0 1 0 5M15 5.5a6.5 6.5 0 0 1 0 9" stroke-linecap="round"/>';
        }

        return '<svg viewBox="0 0 20 20" fill="none" stroke="currentColor" stroke-width="1.4">' +
            '<path d="M4 8v4h2.5L10 15V5L6.5 8H4z" stroke-linejoin="round"/>' + waves + '</svg>';
    }

    slider.addEventListener('input', function () {
        // 送るのは変えた項目だけ。C++側が現在の設定へ混ぜ込む
        sendToGame({ type: 'settingsChanged', audio: { master: Number(slider.value) } });
        render(Number(slider.value));

        // 1目盛りごとに鳴らす。短く連続して鳴らす前提の音なので間引かない。
        // この音そのものが変更後の音量の試聴になる
        sendToGame({ type: 'uiSound', se: 'UiSliderTick' });
    });

    window.onMessageFromGame = function (data) {
        applyDifficultyTheme(data);

        if (!data || data.type !== 'settings' || !data.audio) return;
        if (typeof data.audio.master !== 'number') return;

        render(data.audio.master);
    };

    window.openSettings = function () {
        sendToGame({ type: 'toggleWindow', window: 'settings' });
    };

    render(Number(slider.value));
    sendToGame({ type: 'requestSettings' });
}());
