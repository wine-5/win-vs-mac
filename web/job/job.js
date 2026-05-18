'use strict';

const JOBS = [
    { id: 'Warrior', name: '剣士',    icon: '⚔️',  skill: '全方位斬り',    hp: 1000, atk: 1000, def: 1000, spd: 1000 },
    { id: 'Mage',    name: '魔法使い', icon: '✨',  skill: '巨大魔法弾',    hp:   80, atk:   60, def:   20, spd:   40 },
    { id: 'Ninja',   name: '忍者',    icon: '🥷', skill: '分身一斉攻撃',  hp:   90, atk:   50, def:   25, spd:   55 },
];

let currentJob = null;

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

function onMessageFromGame(data) {
    if (data.type === 'refresh' && data.job != null) {
        currentJob = data.job;
        updateSelection();
    }
}

(function () {
    const list = document.getElementById('job-list');
    JOBS.forEach(function (job) {
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
                '<div class="stat-chip"><span class="k">HP</span> <span class="v">'  + job.hp  + '</span></div>' +
                '<div class="stat-chip"><span class="k">ATK</span><span class="v">' + job.atk + '</span></div>' +
                '<div class="stat-chip"><span class="k">DEF</span><span class="v">' + job.def + '</span></div>' +
                '<div class="stat-chip"><span class="k">SPD</span><span class="v">' + job.spd + '</span></div>' +
            '</div>';
        list.appendChild(card);
    });
}());
