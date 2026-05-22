'use strict';

const JOBS = [
    { id: 'Warrior', name: '剣士',    icon: 'https://assets.game.web/images/ui/select/warrior.png',  skill: '全方位斬り' },
    { id: 'Mage',    name: '魔法使い', icon: 'https://assets.game.web/images/ui/select/mage.png',    skill: '巨大魔法弾' },
    { id: 'Ninja',   name: '忍者',    icon: 'https://assets.game.web/images/ui/select/ninja.png', skill: '分身一斉攻撃' },
];

const JobLogic = (function () {
    let currentJob = null;
    let jobStats = null;
    let onJobSelectedCallback = null;
    let onStatsUpdateCallback = null;
    let onRefreshCallback = null;

    function setCurrentJob(jobId) {
        currentJob = jobId;
        sendToGame({ type: 'jobSelected', job: jobId });
        if (onJobSelectedCallback) onJobSelectedCallback(jobId);
    }

    function setJobStats(stats) {
        jobStats = stats;
        if (onStatsUpdateCallback) onStatsUpdateCallback(stats);
    }

    function setCurrentJobFromRefresh(jobId) {
        currentJob = jobId;
        if (onRefreshCallback) onRefreshCallback(jobId);
    }

    function getCurrentJob() {
        return currentJob;
    }

    function getJobStats() {
        return jobStats;
    }

    function getJobs() {
        return JOBS;
    }

    function getJobById(id) {
        return JOBS.find(function (job) { return job.id === id; });
    }

    function onMessageFromGame(data) {
        if (data.type === 'jobStats') {
            setJobStats(data.stats);
        } else if (data.type === 'refresh' && data.job != null) {
            setCurrentJobFromRefresh(data.job);
        }
    }

    function requestJobStats() {
        sendToGame({ type: 'requestJobStats' });
    }

    function onJobSelected(callback) {
        onJobSelectedCallback = callback;
    }

    function onStatsUpdate(callback) {
        onStatsUpdateCallback = callback;
    }

    function onRefresh(callback) {
        onRefreshCallback = callback;
    }

    return {
        setCurrentJob: setCurrentJob,
        setJobStats: setJobStats,
        setCurrentJobFromRefresh: setCurrentJobFromRefresh,
        getCurrentJob: getCurrentJob,
        getJobStats: getJobStats,
        getJobs: getJobs,
        getJobById: getJobById,
        onMessageFromGame: onMessageFromGame,
        requestJobStats: requestJobStats,
        onJobSelected: onJobSelected,
        onStatsUpdate: onStatsUpdate,
        onRefresh: onRefresh
    };
}());
