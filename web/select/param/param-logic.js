'use strict';

const BAR_CAP = 150;

// 表示する項目のID。C++側から届くキーは base/bonus + 先頭を大文字にしたID。
// 項目を増やすときはここへ足せば、状態の保持も更新も自動で追従する
const STAT_IDS = ['hp', 'atk', 'def', 'spd', 'crit', 'bspd', 'brng'];

const ParamLogic = (function () {
    function toBaseKey(id) {
        return 'base' + id.charAt(0).toUpperCase() + id.slice(1);
    }

    function toBonusKey(id) {
        return 'bonus' + id.charAt(0).toUpperCase() + id.slice(1);
    }

    const state = { slot: null };
    STAT_IDS.forEach(function (id) {
        state[toBaseKey(id)] = 0;
        state[toBonusKey(id)] = 0;
    });

    let onStateChangeCallback = null;
    let firstRender = true;

    function fmt(val) {
        return Number.isInteger(val) ? String(val) : val.toFixed(1);
    }

    function getState() {
        return state;
    }

    function updateState(data) {
        if (data.slot != null) state.slot = data.slot;
        STAT_IDS.forEach(function (id) {
            const baseKey = toBaseKey(id);
            const bonusKey = toBonusKey(id);
            if (data[baseKey] != null) state[baseKey] = data[baseKey];
            if (data[bonusKey] != null) state[bonusKey] = data[bonusKey];
        });

        if (onStateChangeCallback) {
            onStateChangeCallback();
        }
    }

    function calculateBar(baseVal, bonusVal) {
        const base = Math.max(0, baseVal || 0);
        const bonus = Math.max(0, bonusVal || 0);
        const cap = Math.max(BAR_CAP, base + bonus);
        return {
            base: base,
            bonus: bonus,
            cap: cap,
            total: base + bonus,
            basePercent: (base / cap * 100).toFixed(1),
            bonusPercent: (bonus / cap * 100).toFixed(1),
            formattedTotal: fmt(base + bonus)
        };
    }

    function getBarData(id) {
        return calculateBar(state[toBaseKey(id)], state[toBonusKey(id)]);
    }

    function onMessageFromGame(data) {
        if (data.type !== 'refresh') return;
        updateState(data);
    }

    function setFirstRender(value) {
        firstRender = value;
    }

    function isFirstRender() {
        return firstRender;
    }

    function onStateChange(callback) {
        onStateChangeCallback = callback;
    }

    return {
        BAR_CAP: BAR_CAP,
        STAT_IDS: STAT_IDS,
        toBaseKey: toBaseKey,
        toBonusKey: toBonusKey,
        getState: getState,
        updateState: updateState,
        calculateBar: calculateBar,
        getBarData: getBarData,
        fmt: fmt,
        onMessageFromGame: onMessageFromGame,
        setFirstRender: setFirstRender,
        isFirstRender: isFirstRender,
        onStateChange: onStateChange
    };
}());
