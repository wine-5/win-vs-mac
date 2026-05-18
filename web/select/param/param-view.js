'use strict';

const ParamView = (function () {
    let firstRender = true;

    function initialize() {
        ParamLogic.onStateChange(function () {
            renderAll();
        });
        return true;
    }

    function showFloatChange(id, diff) {
        const metricRowElement = document.getElementById('bar-base-' + id);
        if (!metricRowElement) return;
        const containerElement = metricRowElement.closest('.metric-row');
        if (!containerElement) return;
        const floatElement = document.createElement('span');
        floatElement.className = diff > 0 ? 'float-bonus' : 'float-decrease';
        floatElement.textContent = (diff > 0 ? '+' : '') + ParamLogic.fmt(diff);
        containerElement.appendChild(floatElement);
        floatElement.addEventListener('animationend', function () { floatElement.remove(); });
    }

    function flashBar(id, isDecrease) {
        const bar = document.getElementById('bar-base-' + id);
        if (!bar) return;
        bar.classList.remove('bar-flash', 'bar-dim');
        void bar.offsetWidth;
        bar.classList.add(isDecrease ? 'bar-dim' : 'bar-flash');
        bar.addEventListener('animationend', function () {
            bar.classList.remove('bar-flash', 'bar-dim');
        }, { once: true });
    }

    function setBar(id, baseVal, bonusVal) {
        const barData = ParamLogic.calculateBar(baseVal, bonusVal);
        const valEl = document.getElementById('v-' + id);
        const prevTotal = firstRender ? barData.total : (parseFloat(valEl.textContent) || 0);
        const diff = barData.total - prevTotal;

        document.getElementById('bar-base-' + id).style.width = barData.basePercent + '%';
        document.getElementById('bar-bonus-' + id).style.width = barData.bonusPercent + '%';
        valEl.textContent = barData.formattedTotal;

        const bonusEl = document.getElementById('b-' + id);
        if (bonusEl) bonusEl.textContent = barData.bonus > 0 ? ('+' + ParamLogic.fmt(barData.bonus)) : '';

        if (!firstRender && Math.abs(diff) > 0.001) {
            showFloatChange(id, diff);
            flashBar(id, diff < 0);
        }
    }

    function renderAll() {
        const state = ParamLogic.getState();

        setBar('hp', state.baseHp, state.bonusHp);
        setBar('atk', state.baseAtk, state.bonusAtk);
        setBar('def', state.baseDef, state.bonusDef);
        setBar('spd', state.baseSpd, state.bonusSpd);

        const set = function (id, val) {
            const el = document.getElementById(id);
            if (el) el.textContent = val != null ? val : '—';
        };
        set('m-job', state.job);
        set('m-skill', state.skill);
        set('m-slot', state.slot != null && state.slot > 0 ? state.slot + '/3' : null);

        firstRender = false;
        ParamLogic.setFirstRender(false);
    }

    return {
        initialize: initialize,
        renderAll: renderAll,
        setBar: setBar
    };
}());

window.onMessageFromGame = function (data) {
    ParamLogic.onMessageFromGame(data);
};

(function () {
    ParamView.initialize();
}());
