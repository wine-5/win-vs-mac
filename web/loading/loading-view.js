'use strict';

const LoadingView = (function () {
    let cmdBodyEl = null;
    let spinnerEl = null;
    let spinnerTimer = null;
    const SPINNER_FRAMES = ['|', '/', '-', '\\'];
    let spinnerIndex = 0;

    function initialize() {
        cmdBodyEl = document.getElementById('cmd-output');
        if (!cmdBodyEl) return false;

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

        if (type === 'spinner') {
            startSpinner();
            return;
        }

        const lineEl = document.createElement('div');
        lineEl.className = 'cmd-line ' + (type || 'info');
        lineEl.textContent = text;

        cmdBodyEl.appendChild(lineEl);
        cmdBodyEl.scrollTop = cmdBodyEl.scrollHeight;
    }

    function startSpinner() {
        if (spinnerEl) return;

        spinnerEl = document.createElement('div');
        spinnerEl.className = 'cmd-line spinner';
        spinnerEl.textContent = SPINNER_FRAMES[0];
        cmdBodyEl.appendChild(spinnerEl);
        cmdBodyEl.scrollTop = cmdBodyEl.scrollHeight;

        spinnerTimer = setInterval(function () {
            spinnerIndex = (spinnerIndex + 1) % SPINNER_FRAMES.length;
            if (spinnerEl) spinnerEl.textContent = SPINNER_FRAMES[spinnerIndex];
        }, 80);
    }

    function stopSpinner() {
        if (spinnerTimer) {
            clearInterval(spinnerTimer);
            spinnerTimer = null;
        }
        if (spinnerEl) {
            spinnerEl.remove();
            spinnerEl = null;
        }
    }

    function onLoadingComplete() {
        stopSpinner();
        sendToGame({ type: 'loadingComplete' });
    }

    return {
        initialize: initialize,
        addLine: addLine
    };
}());
