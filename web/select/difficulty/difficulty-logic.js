'use strict';

const DIFFICULTY_DESCRIPTIONS = {
    NORMAL: '標準的な難易度。バランスの取れた戦闘が楽しめます。',
    HARD:   '⚠ 上級者向け。敵が強力になり、戦略的なプレイが求められます。'
};

const DifficultyLogic = (function () {
    let currentDiff = 'NORMAL';
    let onDifficultyChangeCallback = null;
    let onHardConfirmCallback = null;

    function selectDifficulty(d) {
        if (d === 'HARD') {
            sendToGame({ type: 'confirmHard' });
            return;
        }
        applyDifficulty(d);
    }

    function applyDifficulty(d) {
        currentDiff = d;
        if (onDifficultyChangeCallback) {
            onDifficultyChangeCallback(d);
        }
        sendToGame({ type: 'difficultyChanged', difficulty: d });
    }

    function getCurrentDifficulty() {
        return currentDiff;
    }

    function getDescription(difficulty) {
        return DIFFICULTY_DESCRIPTIONS[difficulty] || '';
    }

    function confirmHard() {
        applyDifficulty('HARD');
    }

    function onMessageFromGame(data) {
        if (data.type === 'hardConfirmed') {
            confirmHard();
        }
    }

    function onDifficultyChange(callback) {
        onDifficultyChangeCallback = callback;
    }

    return {
        selectDifficulty: selectDifficulty,
        applyDifficulty: applyDifficulty,
        getCurrentDifficulty: getCurrentDifficulty,
        getDescription: getDescription,
        confirmHard: confirmHard,
        onMessageFromGame: onMessageFromGame,
        onDifficultyChange: onDifficultyChange
    };
}());
