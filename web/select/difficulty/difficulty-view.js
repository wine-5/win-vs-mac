'use strict';

const DifficultyView = (function () {
    let btnNormal = null;
    let btnHard = null;
    let descEl = null;

    function initialize() {
        btnNormal = document.getElementById('btn-normal');
        btnHard = document.getElementById('btn-hard');
        descEl = document.getElementById('diff-desc');

        if (!btnNormal || !btnHard || !descEl) return false;

        DifficultyLogic.onDifficultyChange(function (difficulty) {
            updateDisplay(difficulty);
        });

        return true;
    }

    function updateDisplay(difficulty) {
        if (!btnNormal || !btnHard || !descEl) return;

        btnNormal.className = 'diff-btn';
        btnHard.className = 'diff-btn danger';

        if (difficulty === 'NORMAL') {
            btnNormal.classList.add('active-normal');
        } else if (difficulty === 'HARD') {
            btnHard.classList.add('active-hard');
        }

        descEl.textContent = DifficultyLogic.getDescription(difficulty);
        descEl.className = difficulty === 'HARD' ? 'diff-desc warn' : 'diff-desc';
    }

    return {
        initialize: initialize,
        updateDisplay: updateDisplay
    };
}());

window.onMessageFromGame = function (data) {
    DifficultyLogic.onMessageFromGame(data);
};

function selectDiff(difficulty) {
    DifficultyLogic.selectDifficulty(difficulty);
}

(function () {
    if (DifficultyView.initialize()) {
        DifficultyView.updateDisplay(DifficultyLogic.getCurrentDifficulty());
    }
}());
