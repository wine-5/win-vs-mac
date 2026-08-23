/**
 * ゲームパッドの操作を、ページ内のフォーカス移動へ翻訳する
 *
 * セレクト画面とリザルト画面は WebView2 の別ウィンドウで、DxLib の入力ループの外にある。
 * OS のカーソルを合成で動かす手もあるが、ゲームが前面を失った瞬間に他のアプリへ
 * 入力が漏れるため採らない。代わりに C++ 側がパッドを読み、
 * {type:'pad', action:'...'} を postMessage で送ってくるのをここで受ける。
 *
 * messaging.js の onMessageFromGame には相乗りせず、自前でリスナーを足す。
 * ページごとのロジックと取り合いにならないようにするため。
 */
(function () {
    'use strict';

    if (!window.chrome || !window.chrome.webview) return;

    // 押せるものの拾い方。ページによって button だったり onclick を持つ div だったりする。
    // 生成した要素には data-pad-focus を付けてもらう（属性の onclick と違い、
    // JS で onclick を代入したものは属性セレクタで拾えないため）
    var SELECTOR = [
        '[data-pad-focus]',
        'button:not([disabled])',
        'input:not([type="hidden"]):not([disabled])',
        'select:not([disabled])',
        'textarea:not([disabled])',
        'a[href]',
        '[onclick]',
        '[ondblclick]'
    ].join(',');

    // 方向で移動先を探すときに、横へのずれをどれだけ嫌うか。
    // 1 だと斜めの要素へも同じ重みで飛ぶので、真っすぐ近いものを優先させる
    var CROSS_PENALTY = 3;

    var current = null;

    /** 選択枠のスタイルを流し込む（各ページの CSS を触らずに済ませる） */
    function injectStyle() {
        var style = document.createElement('style');
        style.textContent =
            '.pad-focus{outline:3px solid #4cc2ff !important;' +
            'outline-offset:2px;border-radius:4px;}';
        document.head.appendChild(style);
    }

    /** いま画面に出ていて押せるものだけを集める */
    function candidates() {
        var list = [];
        var nodes = document.querySelectorAll(SELECTOR);
        for (var i = 0; i < nodes.length; i++) {
            var el = nodes[i];
            if (el.disabled) continue;

            // 表示されていないものは飛ばす。offsetParent は display:none の祖先も見てくれる
            if (el.offsetParent === null) continue;

            var rect = el.getBoundingClientRect();
            if (rect.width <= 0 || rect.height <= 0) continue;

            list.push(el);
        }
        return list;
    }

    /** 要素の中心座標 */
    function centerOf(el) {
        var rect = el.getBoundingClientRect();
        return { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
    }

    /** 枠を移す */
    function setFocus(el) {
        if (current === el) return;

        if (current) current.classList.remove('pad-focus');
        current = el;
        if (!current) return;

        current.classList.add('pad-focus');
        if (typeof current.focus === 'function') current.focus({ preventScroll: true });
        if (typeof current.scrollIntoView === 'function') {
            current.scrollIntoView({ block: 'nearest', inline: 'nearest' });
        }
    }

    /** 枠がどこにも無ければ先頭へ置く */
    function ensureFocus() {
        var list = candidates();
        if (list.length === 0) return false;

        // 前に選んでいたものが消えている（一覧が組み直された）ことがある
        if (current && list.indexOf(current) >= 0) return true;

        setFocus(list[0]);
        return true;
    }

    /** 指定方向にある一番近いものへ移す */
    function move(dirX, dirY) {
        if (!ensureFocus()) return;

        var list = candidates();
        var from = centerOf(current);

        var best = null;
        var bestScore = 0;
        for (var i = 0; i < list.length; i++) {
            var el = list[i];
            if (el === current) continue;

            var to = centerOf(el);
            var deltaX = to.x - from.x;
            var deltaY = to.y - from.y;

            // 押した方向にあるものだけを候補にする
            var along = dirX !== 0 ? deltaX * dirX : deltaY * dirY;
            if (along <= 0) continue;

            var cross = dirX !== 0 ? Math.abs(deltaY) : Math.abs(deltaX);
            var score = along + cross * CROSS_PENALTY;

            if (best !== null && score >= bestScore) continue;

            best = el;
            bestScore = score;
        }

        if (best) setFocus(best);
    }

    /** いま枠が当たっているものを押す */
    function activate() {
        if (!ensureFocus()) return;

        // デスクトップのアイコンはダブルクリックで開く作りなので、
        // 単クリックを持たないものにはダブルクリックを送る
        var hasClick = current.onclick || current.hasAttribute('onclick');
        if (!hasClick && current.hasAttribute('ondblclick')) {
            current.dispatchEvent(new MouseEvent('dblclick', { bubbles: true, cancelable: true }));
            return;
        }

        if (typeof current.click === 'function') current.click();
    }

    /** 枠を消す（別のウィンドウへ操作が移ったとき） */
    function blur() {
        if (!current) return;

        current.classList.remove('pad-focus');
        current = null;
    }

    function handle(action) {
        switch (action) {
            case 'up': move(0, -1); break;
            case 'down': move(0, 1); break;
            case 'left': move(-1, 0); break;
            case 'right': move(1, 0); break;
            case 'confirm': activate(); break;
            case 'focus': ensureFocus(); break;
            case 'blur': blur(); break;
            default: break;
        }
    }

    injectStyle();

    window.chrome.webview.addEventListener('message', function (e) {
        try {
            var data = JSON.parse(e.data);
            if (!data || data.type !== 'pad') return;

            handle(data.action);
        } catch (err) {
            console.error('pad-nav error:', err);
        }
    });
})();
