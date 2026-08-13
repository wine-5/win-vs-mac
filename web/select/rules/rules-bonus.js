'use strict';

/**
 * 装備ページの「どの拡張子で何が伸びるか」を実データから作る。
 *
 * 値は extensionBonus.json をそのまま読む。これは C++ の
 * ExtensionBonusRepository が読むのと同一ファイルなので、バランス調整で
 * 数値を変えても説明が古くならない。HTMLへ数値を直接書かないのはこのため。
 */
(function () {
    const BONUS_JSON_URL = 'https://assets.game.web/data/extensionBonus.json';

    // JSONの項目名 → STAT_META のキー。C++（FileSelectWindow の STAT_FIELDS）と同じ対応
    const FIELD_TO_STAT = {
        hp: 'hp',
        atk: 'atk',
        def: 'def',
        spd: 'spd',
        attackRange: 'rng',
        criticalRate: 'crit',
        projectileSpeed: 'bspd',
        projectileRange: 'brng'
    };

    // クリティカル率だけは確率（0.08）で持っているので、見せるときに%へ直す
    const PERCENT_SCALE = 100;

    // 表示順はパラメータウィンドウのステータス順に合わせる。
    // 複数を伸ばす種別でも並びが毎回同じになり、行同士を見比べられる
    const FIELD_ORDER = ['hp', 'atk', 'def', 'spd', 'attackRange',
                         'criticalRate', 'projectileSpeed', 'projectileRange'];

    /** 小数の末尾が0なら整数で見せる（+90.0 ではなく +90 と書く） */
    function formatValue(value) {
        const rounded = Math.round(value * 10) / 10;
        return Number.isInteger(rounded) ? String(rounded) : rounded.toFixed(1);
    }

    function buildGain(entry) {
        let html = '';
        FIELD_ORDER.forEach(function (field) {
            const raw = entry[field];
            if (typeof raw !== 'number' || raw === 0) return;

            const statId = FIELD_TO_STAT[field];
            const meta = STAT_META[statId];
            if (!meta) return;

            const value = (statId === 'crit') ? raw * PERCENT_SCALE : raw;
            html += '<span class="gain-item">' +
                    '<img class="stat-icon" src="' + meta.icon + '" alt="">' +
                    '<span class="stat-name">' + meta.name + '</span>' +
                    '<span class="stat-value">+' + formatValue(value) + meta.suffix + '</span>' +
                    '</span>';
        });
        return html;
    }

    fetch(BONUS_JSON_URL).then(function (res) {
        return res.json();
    }).then(function (json) {
        const bonuses = json.bonuses || {};
        document.querySelectorAll('#ext-grid .ext-row').forEach(function (row) {
            const entry = bonuses[row.dataset.key];
            const gain = row.querySelector('.ext-gain');
            if (!entry || !gain) return;

            gain.innerHTML = buildGain(entry);
        });
    }).catch(function () {
        // 読めなくても説明の他の部分は読ませたいので、行は空のままにして黙って続ける
    });
}());
