'use strict';

const LoadingView = (function () {
    let cmdBodyEl = null;

    function initialize() {
        cmdBodyEl = document.getElementById('cmd-output');
        if (!cmdBodyEl) return false;

        // ロジックのコールバックを設定
        LoadingLogic.onLineReady(function (text, type) {
            addLine(text, type);
        });

        LoadingLogic.onLoadingComplete(function () {
            onLoadingComplete();
        });

        return true;
    }

    function addLine(text, type) {
        if (!cmdBodyEl) return;

        const lineEl = document.createElement('div');
        lineEl.className = 'cmd-line ' + (type || 'info');
        lineEl.textContent = text;

        cmdBodyEl.appendChild(lineEl);

        // 自動スクロール：最後の行が見えるようにスクロール
        cmdBodyEl.scrollTop = cmdBodyEl.scrollHeight;
    }

    function onLoadingComplete() {
        // ローディング完了をC++に通知
        sendToGame({ type: 'loadingComplete' });
    }

    return {
        initialize: initialize,
        addLine: addLine
    };
}());
