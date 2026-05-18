'use strict';

// 表示専用データ（名前・アイコン・スキル名はラベルのため JS 側で保持）
// hp/atk/def/spd は C++ から jobStats メッセージで受け取る
const JOBS = [
    { id: 'Warrior', name: '剣士',    icon: '⚔️',  skill: '全方位斬り' },
    { id: 'Mage',    name: '魔法使い', icon: '✨',  skill: '巨大魔法弾' },
    { id: 'Ninja',   name: '忍者',    icon: '🥷', skill: '分身一斉攻撃' },
];

let currentJob = null;
let jobStats = null; // C++ から受け取るステータス配列 [{ id, hp, atk, def, spd }, ...]

function selectJob(jobId) {
    currentJob = jobId;
    updateSelection();
    sendToGame({ type: 'jobSelected', job: jobId });
}

function updateSelection() {
    document.querySelectorAll('.job-card').forEach(function (card) {
        card.classList.toggle('active', card.dataset.id === currentJob);
    });
}

function renderJobs() {
    const list = document.getElementById('job-list');
    list.innerHTML = '';
    JOBS.forEach(function (job, i) {
        const stats = jobStats ? jobStats[i] : null;
        const card = document.createElement('div');
        card.className = 'job-card';
        card.dataset.id = job.id;
        card.onclick = function () { selectJob(job.id); };
        card.innerHTML =
            '<div class="job-header">' +
                '<span class="job-icon">' + job.icon + '</span>' +
                '<div class="job-info">' +
                    '<div class="job-name">' + job.name + '</div>' +
                    '<div class="job-skill">Skill: ' + job.skill + '</div>' +
                '</div>' +
            '</div>' +
            '<div class="job-stats">' +
                '<div class="stat-chip"><span class="k">HP</span> <span class="v stat-hp">'  + (stats ? stats.hp  : '—') + '</span></div>' +
                '<div class="stat-chip"><span class="k">ATK</span><span class="v stat-atk">' + (stats ? stats.atk : '—') + '</span></div>' +
                '<div class="stat-chip"><span class="k">DEF</span><span class="v stat-def">' + (stats ? stats.def : '—') + '</span></div>' +
                '<div class="stat-chip"><span class="k">SPD</span><span class="v stat-spd">' + (stats ? stats.spd : '—') + '</span></div>' +
            '</div>';
        list.appendChild(card);
    });
    updateSelection();
}

function onMessageFromGame(data) {
    if (data.type === 'jobStats') {
        jobStats = data.stats;
        renderJobs();
    } else if (data.type === 'refresh' && data.job != null) {
        currentJob = data.job;
        updateSelection();
    }
}

(function () {
    renderJobs();
    sendToGame({ type: 'requestJobStats' });
}());
