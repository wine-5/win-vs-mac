'use strict';

/**
 * 設定ウィンドウの表示とDOM操作。値の保持は SettingsLogic が持つ
 */
(function () {
    // 効果音のスライダーを動かしたときに鳴らす試聴音の最短間隔（ミリ秒）。
    // 1目盛りごとに鳴らすとドラッグ中に音が重なって潰れ、かえって音量が分からなくなる
    const PREVIEW_INTERVAL_MS = 90;

    const sliders = Array.prototype.slice.call(document.querySelectorAll('input[type=range]'));
    const toggles = Array.prototype.slice.call(document.querySelectorAll('.toggle'));

    let lastPreviewTime = 0;

    /**
     * 画面全体を現在の設定で描き直す
     * @param {object} settings 現在の設定
     */
    function render(settings) {
        const values = Object.assign({}, settings.audio, settings.control);

        sliders.forEach(function (slider) {
            const value = values[slider.dataset.key];
            slider.value = value;

            const ratio = (value - slider.min) / (slider.max - slider.min) * 100;
            slider.style.setProperty('--fill', ratio + '%');

            document.querySelector('[data-for="' + slider.dataset.key + '"]').textContent = value;
        });

        toggles.forEach(function (toggle) {
            const isOn = Boolean(values[toggle.dataset.key]);
            toggle.classList.toggle('is-on', isOn);
            document.querySelector('.toggle-label[data-for="' + toggle.dataset.key + '"]').textContent =
                isOn ? 'オン' : 'オフ';
        });

        renderMasterIcon(settings.audio.master);
    }

    /**
     * マスター音量の値でスピーカーの波の数を変える
     *
     * ミュート専用のボタンは置かない。0にすれば同じことなので、
     * 押せる場所を増やさずアイコンの見た目だけで状態を伝える
     * @param {number} master マスター音量
     */
    function renderMasterIcon(master) {
        let waves;
        if (master <= 0) {
            waves = '<path d="M13 7.5 17 12.5M17 7.5 13 12.5" stroke-linecap="round"/>';
        } else if (master < 50) {
            waves = '<path d="M13 7.5a3.5 3.5 0 0 1 0 5" stroke-linecap="round"/>';
        } else {
            waves = '<path d="M13 7.5a3.5 3.5 0 0 1 0 5M15 5.5a6.5 6.5 0 0 1 0 9" stroke-linecap="round"/>';
        }

        document.getElementById('icon-master').innerHTML =
            '<svg viewBox="0 0 20 20" fill="none" stroke="currentColor" stroke-width="1.4">' +
            '<path d="M4 8v4h2.5L10 15V5L6.5 8H4z" stroke-linejoin="round"/>' + waves + '</svg>';
    }

    /**
     * 効果音の音量を変えたときだけ、その音量で試聴音を鳴らす
     *
     * 音量は鳴らさないと分からないので、変更操作そのものを試聴にしてしまう
     */
    function playSePreview() {
        const now = Date.now();
        if (now - lastPreviewTime < PREVIEW_INTERVAL_MS) return;

        lastPreviewTime = now;
        sendToGame({ type: 'uiSound', se: 'UiKeyPress' });
    }

    sliders.forEach(function (slider) {
        slider.addEventListener('input', function () {
            SettingsLogic.setValue(slider.dataset.key, Number(slider.value));

            if (slider.dataset.key === 'se' || slider.dataset.key === 'master')
                playSePreview();
        });
    });

    toggles.forEach(function (toggle) {
        toggle.addEventListener('click', function () {
            SettingsLogic.setValue(toggle.dataset.key, !toggle.classList.contains('is-on'));
        });
    });

    document.querySelectorAll('[data-reset]').forEach(function (button) {
        button.addEventListener('click', function () {
            SettingsLogic.resetPage(button.dataset.reset);
        });
    });

    // 左ナビでページを切り替える
    document.querySelectorAll('.nav-item').forEach(function (item) {
        item.addEventListener('click', function () {
            document.querySelectorAll('.nav-item').forEach(function (other) {
                other.classList.toggle('is-active', other === item);
            });
            document.querySelectorAll('.page').forEach(function (page) {
                page.classList.toggle('is-active', page.id === 'page-' + item.dataset.page);
            });
            document.querySelector('.content').scrollTop = 0;
        });
    });

    SettingsLogic.onChange(render);
    render(SettingsLogic.getSettings());

    // 開いた時点の値をC++へ要求する（保存済みの値で表示を合わせる）
    sendToGame({ type: 'requestSettings' });
}());
