'use strict';

const ResultLogic = (function () {
    let onResultDataCallback = null;

    function formatTime(seconds) {
        const m = Math.floor(seconds / 60);
        const s = Math.floor(seconds % 60);
        return m + ':' + String(s).padStart(2, '0');
    }

    /**
     * 大きく見せる用に mm:ss へ 0 埋めして整える
     * @param {number} seconds 秒数
     * @returns {string} "04:19" 形式の文字列
     */
    function formatClock(seconds) {
        const m = Math.floor((seconds || 0) / 60);
        const s = Math.floor((seconds || 0) % 60);
        return String(m).padStart(2, '0') + ':' + String(s).padStart(2, '0');
    }

    function escapeHtml(str) {
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;');
    }

    // ランクはクリアタイムだけで決まるタイムアタック評価。
    // 被ダメージを条件に入れていた頃は、敵の一撃（実ダメージ14〜78）を2発もらった
    // 時点でC以下が確定し、S/Aは無被弾必須で事実上出せなかった。
    //
    // 秒数は「その難易度で普通にクリアしたときのタイム」を基準に置いている。
    // NORMAL は 2:00、HARD は 4:00 を S のラインとする。
    const RANK_THRESHOLDS = {
        NORMAL: { S: 120, A: 165, B: 220, C: 300 },
        HARD:   { S: 240, A: 330, B: 440, C: 600 }
    };

    const RANK_ORDER = ['S', 'A', 'B', 'C', 'D'];

    /**
     * 難易度に対応するタイム閾値を返す
     * @param {object} data リザルトデータ
     * @returns {object} ランクごとの秒数
     */
    function thresholdsOf(data) {
        return RANK_THRESHOLDS[data && data.difficulty === 'HARD' ? 'HARD' : 'NORMAL'];
    }

    /**
     * クリアタイムからランクを求める
     * @param {object} data リザルトデータ
     * @returns {string} 'S'|'A'|'B'|'C'|'D'
     */
    function calcRank(data) {
        const t = (data && data.elapsedTime) || 0;
        const th = thresholdsOf(data);
        if (t <= th.S) return 'S';
        if (t <= th.A) return 'A';
        if (t <= th.B) return 'B';
        if (t <= th.C) return 'C';
        return 'D';
    }

    /**
     * ひとつ上のランクに必要な短縮秒数を求める
     * @param {object} data リザルトデータ
     * @returns {?object} { rank, remainSeconds }。Sランクなら null
     */
    function calcNextRankGoal(data) {
        const rank = calcRank(data);
        if (rank === 'S') return null;

        const nextRank = RANK_ORDER[RANK_ORDER.indexOf(rank) - 1];
        const th = thresholdsOf(data);
        return {
            rank: nextRank,
            remainSeconds: Math.max(0, ((data && data.elapsedTime) || 0) - th[nextRank])
        };
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
        formatClock: formatClock,
        escapeHtml: escapeHtml,
        calcRank: calcRank,
        calcNextRankGoal: calcNextRankGoal,
        onMessageFromGame: onMessageFromGame,
        requestResult: requestResult,
        onResultData: onResultData
    };
}());
