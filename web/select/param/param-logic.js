'use strict';

const BAR_CAP = 150;

const ParamLogic = (function () {
    const state = {
        slot: null,
        baseHp: 0, baseAtk: 0, baseDef: 0, baseSpd: 0,
        bonusHp: 0, bonusAtk: 0, bonusDef: 0, bonusSpd: 0,
    };

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
        if (data.baseHp != null) state.baseHp = data.baseHp;
        if (data.baseAtk != null) state.baseAtk = data.baseAtk;
        if (data.baseDef != null) state.baseDef = data.baseDef;
        if (data.baseSpd != null) state.baseSpd = data.baseSpd;
        if (data.bonusHp != null) state.bonusHp = data.bonusHp;
        if (data.bonusAtk != null) state.bonusAtk = data.bonusAtk;
        if (data.bonusDef != null) state.bonusDef = data.bonusDef;
        if (data.bonusSpd != null) state.bonusSpd = data.bonusSpd;

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
        const baseKey = 'base' + id.charAt(0).toUpperCase() + id.slice(1);
        const bonusKey = 'bonus' + id.charAt(0).toUpperCase() + id.slice(1);
        return calculateBar(state[baseKey], state[bonusKey]);
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
