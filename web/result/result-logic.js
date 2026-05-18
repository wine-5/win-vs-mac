'use strict';

const ResultLogic = (function () {
    let onResultDataCallback = null;

    function formatTime(seconds) {
        const m = Math.floor(seconds / 60);
        const s = Math.floor(seconds % 60);
        return m + ':' + String(s).padStart(2, '0');
    }

    function escapeHtml(str) {
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;');
    }

    function calcRank(data) {
        const dmg = data.totalDamageTaken || 0;
        const t = data.elapsedTime || 0;
        if (dmg === 0 && t < 30) return 'S';
        if (dmg === 0 && t < 120) return 'A';
        if (dmg < 50) return 'B';
        if (dmg < 150) return 'C';
        return 'D';
    }

    function onMessageFromGame(data) {
        if (data.type === 'resultData') {
            if (onResultDataCallback) {
                onResultDataCallback(data);
            }
        }
    }

    function requestResult() {
        sendToGame({ type: 'requestResult' });
    }

    function onResultData(callback) {
        onResultDataCallback = callback;
    }

    return {
        formatTime: formatTime,
        escapeHtml: escapeHtml,
        calcRank: calcRank,
        onMessageFromGame: onMessageFromGame,
        requestResult: requestResult,
        onResultData: onResultData
    };
}());
