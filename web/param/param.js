'use strict';

const BAR_CAP = 150;
let firstRender = true;

const state = {
    job: null, skill: null, slot: null,
    baseHp: 0, baseAtk: 0, baseDef: 0, baseSpd: 0,
    bonusHp: 0, bonusAtk: 0, bonusDef: 0, bonusSpd: 0,
};

document.addEventListener('DOMContentLoaded', function () {
    document.querySelectorAll('.tab').forEach(function (tab) {
        tab.addEventListener('click', function () {
            document.querySelectorAll('.tab').forEach(function (t) { t.classList.remove('active'); });
            tab.classList.add('active');
            const isPerf = tab.dataset.tab === 'perf';
            document.getElementById('perf-view').classList.toggle('hidden', !isPerf);
            document.getElementById('detail-view').classList.toggle('visible', !isPerf);
        });
    });
});

function fmt(val) {
    return Number.isInteger(val) ? String(val) : val.toFixed(1);
}

function showFloatChange(id, diff) {
    const metricRow = document.getElementById('bar-base-' + id);
    if (!metricRow) return;
    const container = metricRow.closest('.metric-row');
    if (!container) return;
    const el = document.createElement('span');
    el.className = diff > 0 ? 'float-bonus' : 'float-decrease';
    el.textContent = (diff > 0 ? '+' : '') + fmt(diff);
    container.appendChild(el);
    el.addEventListener('animationend', function () { el.remove(); });
}

function flashBar(id, isDecrease) {
    const bar = document.getElementById('bar-base-' + id);
    if (!bar) return;
    bar.classList.remove('bar-flash', 'bar-dim');
    void bar.offsetWidth; // reflow to restart animation
    bar.classList.add(isDecrease ? 'bar-dim' : 'bar-flash');
    bar.addEventListener('animationend', function () {
        bar.classList.remove('bar-flash', 'bar-dim');
    }, { once: true });
}

function setBar(id, baseVal, bonusVal) {
    const base  = Math.max(0, baseVal  || 0);
    const bonus = Math.max(0, bonusVal || 0);
    const cap   = Math.max(BAR_CAP, base + bonus);
    const newTotal = base + bonus;

    const valEl = document.getElementById('v-' + id);
    const prevTotal = firstRender ? newTotal : (parseFloat(valEl.textContent) || 0);
    const diff = newTotal - prevTotal;

    document.getElementById('bar-base-'  + id).style.width = (base  / cap * 100).toFixed(1) + '%';
    document.getElementById('bar-bonus-' + id).style.width = (bonus / cap * 100).toFixed(1) + '%';
    valEl.textContent = fmt(newTotal);

    const bonusEl = document.getElementById('b-' + id);
    if (bonusEl) bonusEl.textContent = bonus > 0 ? ('+' + fmt(bonus)) : '';

    if (!firstRender && Math.abs(diff) > 0.001) {
        showFloatChange(id, diff);
        flashBar(id, diff < 0);
    }
}

function renderAll() {
    setBar('hp',  state.baseHp,  state.bonusHp);
    setBar('atk', state.baseAtk, state.bonusAtk);
    setBar('def', state.baseDef, state.bonusDef);
    setBar('spd', state.baseSpd, state.bonusSpd);

    const set = function (id, val) {
        const el = document.getElementById(id);
        if (el) el.textContent = val != null ? val : '—';
    };
    set('m-job',   state.job);
    set('m-skill', state.skill);
    set('m-slot',  state.slot != null && state.slot > 0 ? state.slot + '/3' : null);

    firstRender = false;
}

function onMessageFromGame(data) {
    if (data.type !== 'refresh') return;
    if (data.job      != null) state.job      = data.job;
    if (data.skill    != null) state.skill    = data.skill;
    if (data.slot     != null) state.slot     = data.slot;
    if (data.baseHp   != null) state.baseHp   = data.baseHp;
    if (data.baseAtk  != null) state.baseAtk  = data.baseAtk;
    if (data.baseDef  != null) state.baseDef  = data.baseDef;
    if (data.baseSpd  != null) state.baseSpd  = data.baseSpd;
    if (data.bonusHp  != null) state.bonusHp  = data.bonusHp;
    if (data.bonusAtk != null) state.bonusAtk = data.bonusAtk;
    if (data.bonusDef != null) state.bonusDef = data.bonusDef;
    if (data.bonusSpd != null) state.bonusSpd = data.bonusSpd;
    renderAll();
}
