'use strict';

const JobView = (function () {
    let jobListEl = null;
    let currentHighlight = null;

    function initialize() {
        jobListEl = document.getElementById('job-list');
        if (!jobListEl) return false;

        JobLogic.onJobSelected(function (jobId) {
            updateSelection(jobId);
        });

        JobLogic.onStatsUpdate(function () {
            render();
        });

        JobLogic.onRefresh(function (jobId) {
            updateSelection(jobId);
        });

        return true;
    }

    function render() {
        if (!jobListEl) return;
        jobListEl.innerHTML = '';

        const jobs = JobLogic.getJobs();
        const stats = JobLogic.getJobStats();
        const currentJob = JobLogic.getCurrentJob();

        jobs.forEach(function (job, i) {
            const jobStats = stats ? stats[i] : null;
            const card = document.createElement('div');
            card.className = 'job-card';
            card.dataset.id = job.id;
            card.innerHTML =
                '<div class="job-header">' +
                    '<img class="job-icon" src="' + job.icon + '" alt="' + job.name + '">' +
                    '<div class="job-info">' +
                        '<div class="job-name">' + job.name + '</div>' +
                        '<div class="job-skill">Skill: ' + job.skill + '</div>' +
                    '</div>' +
                '</div>' +
                '<div class="job-stats">' +
                    '<div class="stat-chip"><span class="k">HP</span> <span class="v stat-hp">'  + (jobStats ? jobStats.hp  : '—') + '</span></div>' +
                    '<div class="stat-chip"><span class="k">ATK</span><span class="v stat-atk">' + (jobStats ? jobStats.atk : '—') + '</span></div>' +
                    '<div class="stat-chip"><span class="k">DEF</span><span class="v stat-def">' + (jobStats ? jobStats.def : '—') + '</span></div>' +
                    '<div class="stat-chip"><span class="k">SPD</span><span class="v stat-spd">' + (jobStats ? jobStats.spd : '—') + '</span></div>' +
                '</div>';

            card.onclick = function () {
                JobLogic.setCurrentJob(job.id);
            };

            jobListEl.appendChild(card);
        });

        updateSelection(currentJob);
    }

    function updateSelection(jobId) {
        const cards = jobListEl ? jobListEl.querySelectorAll('.job-card') : [];
        cards.forEach(function (card) {
            const isActive = card.dataset.id === jobId;
            card.classList.toggle('active', isActive);
        });
    }

    return {
        initialize: initialize,
        render: render,
        updateSelection: updateSelection
    };
}());

window.onMessageFromGame = function (data) {
    JobLogic.onMessageFromGame(data);
};

(function () {
    if (JobView.initialize()) {
        JobView.render();
        JobLogic.requestJobStats();
    }
}());
