'use strict';

/**
 * 画面上の操作に効果音を返す共通の仕掛け。
 *
 * 押した要素ごとに個別のコードを書くと、ボタンを増やすたびに鳴らし忘れる。
 * ここで画面全体のクリックを拾い、操作できる要素なら音を要求する形にしてある。
 * 実際に鳴らすのはC++（platform::window::tryPlayUiSound）で、
 * 音源と音量は resources.json が持つ。
 *
 * SEを変えたい要素には data-se="UiKeyPress" のように書けば個別に指定できる。
 */
(function () {
    // 押せるもの。ここに載っていない要素（見出し・本文）を押しても鳴らさない
    const INTERACTIVE = 'button, input, label, select, .menu-item, .desk-icon,' +
                        ' .taskbar-app, .file-row, [onclick], [ondblclick], [role="button"]';

    // ダブルクリックで動く要素は、その手前で鳴るシングルクリックの音を抑える。
    // 抑えないと1回の操作で3回鳴ってしまう
    const DOUBLE_CLICK_ONLY = '[ondblclick]';

    const DEFAULT_SE = 'UiClick';

    function play(element) {
        const se = element.dataset && element.dataset.se ? element.dataset.se : DEFAULT_SE;
        sendToGame({ type: 'uiSound', se: se });
    }

    document.addEventListener('click', function (event) {
        const target = event.target.closest(INTERACTIVE);
        if (!target) return;

        // ダブルクリックで動く要素は dblclick 側で鳴らす
        if (target.closest(DOUBLE_CLICK_ONLY)) return;

        play(target);
    }, true);

    document.addEventListener('dblclick', function (event) {
        const target = event.target.closest(DOUBLE_CLICK_ONLY);
        if (target) play(target);
    }, true);
}());
