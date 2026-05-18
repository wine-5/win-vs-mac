'use strict';

const BAR_CAP = 150;

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

function setBar(id, baseVal, bonusVal) {
    const base  = Math.max(0, baseVal  || 0);
    const bonus = Math.max(0, bonusVal || 0);
    const cap   = Math.max(BAR_CAP, base + bonus);
    document.getElementById('bar-base-'  + id).style.width = (base  / cap * 100).toFixed(1) + '%';
    document.getElementById('bar-bonus-' + id).style.width = (bonus / cap * 100).toFixed(1) + '%';
    document.getElementById('v-' + id).textContent = base + bonus;
    const bonusEl = document.getElementById('b-' + id);
    if (bonusEl) bonusEl.textContent = bonus > 0 ? ('+' + bonus) : '';
}

function renderAll() {
    setBar('hp',  state.baseHp,  state.bonusHp);
    setBar('atk', state.baseAtk, state.bonusAtk);
    setBar('def', state.baseDef, state.bonusDef);
    setBar('spd', state.baseSpd, state.bonusSpd);
    const set = function (id, val) {
        const el = document.getElementById(id);
        if (el) el.textContent = val != null ? val : '\u2014';
    };
    set('m-job',   state.job);
    set('m-skill', state.skill);
    set('m-slot',  state.slot != null ? 'Slot ' + (state.slot + 1) : null);
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