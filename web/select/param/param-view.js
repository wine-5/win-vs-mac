'use strict';

const ParamView = (function () {
    let firstRender = true;
    let prevJob = null;

    function initialize() {
        ParamLogic.onStateChange(function () {
            renderAll();
        });
        return true;
    }

    /** Job が切り替わったとき: バーを 0 リセット → 時間差でスライドイン + アニメーション */
    function animateJobEntrance(state) {
        const stats = [
            { id: 'hp',  base: state.baseHp,  bonus: state.bonusHp  },
            { id: 'atk', base: state.baseAtk, bonus: state.bonusAtk },
            { id: 'def', base: state.baseDef, bonus: state.bonusDef },
            { id: 'spd', base: state.baseSpd, bonus: state.bonusSpd },
        ];

        stats.forEach(function (s, i) {
            const baseBar  = document.getElementById('bar-base-'  + s.id);
            const bonusBar = document.getElementById('bar-bonus-' + s.id);
            const valEl    = document.getElementById('v-' + s.id);
            const bonusEl  = document.getElementById('b-' + s.id);
            const row      = baseBar ? baseBar.closest('.metric-row') : null;

            // 瞬時にリセット（transition を一時無効化）
            if (baseBar)  { baseBar.style.transition  = 'none'; baseBar.style.width  = '0%'; }
            if (bonusBar) { bonusBar.style.transition = 'none'; bonusBar.style.width = '0%'; }
            if (valEl)    { valEl.textContent = '0'; }
            if (bonusEl)  { bonusEl.textContent = ''; }
            if (row) {
                row.style.transition = 'none';
                row.style.opacity    = '0';
                row.style.transform  = 'translateX(-10px)';
            }

            // 行ごとに時間差でスライドイン
            setTimeout(function () {
                if (row) {
                    row.style.transition = 'opacity 0.25s ease-out, transform 0.25s ease-out';
                    row.style.opacity    = '1';
                    row.style.transform  = 'translateX(0)';
                }
                // CSS transition を復元してからバー幅を設定 → 0% → target% がアニメーション
                if (baseBar)  { baseBar.style.transition  = ''; }
                if (bonusBar) { bonusBar.style.transition = ''; }
                setBar(s.id, s.base, s.bonus);
            }, 60 + i * 130);
        });
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
        const barElement = document.getElementById('bar-base-' + id);
        if (!barElement) return;
        barElement.classList.remove('bar-flash', 'bar-dim');
        void barElement.offsetWidth;
        barElement.classList.add(isDecrease ? 'bar-dim' : 'bar-flash');
        barElement.addEventListener('animationend', function () {
            barElement.classList.remove('bar-flash', 'bar-dim');
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
        const jobChanged = state.job !== prevJob && state.job != null;
        prevJob = state.job;

        if (jobChanged) {
            // Job が変わったとき: 時間差スライドイン演出
            animateJobEntrance(state);
        } else {
            setBar('hp',  state.baseHp,  state.bonusHp);
            setBar('atk', state.baseAtk, state.bonusAtk);
            setBar('def', state.baseDef, state.bonusDef);
            setBar('spd', state.baseSpd, state.bonusSpd);
        }

        const set = function (id, val) {
            const el = document.getElementById(id);
            if (el) el.textContent = val != null ? val : '—';
        };
        set('m-job', state.job);
        set('top-job', state.job);
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
