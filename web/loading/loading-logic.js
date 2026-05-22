'use strict';

// ローディング中に表示するメッセージ
const LOADING_MESSAGES = [
    { text: '  WIN vs MAC.exe v1.0.0  ',                                              type: 'info',    delay: 0.00 },
    { text: '  Copyright (C) 2026  WIN vs MAC wine-5',                                type: 'info',    delay: 0.08 },
    { text: '----------------------------------------------------------------------', type: 'info',    delay: 0.08 },
    { text: '',                                                                       type: 'info',    delay: 0.32 },
    { text: 'C:\Users\Win VS Mac.exe> start Game!!!',                                 type: 'info',    delay: 0.50 },
    { text: '',                                                                       type: 'spinner', delay: 0.10 },
    { text: 'Initializing Dungeon Environment...',                                    type: 'info',    delay: 1.00 },
    { text: '  Service Locator ........................................ [OK]',        type: 'success', delay: 0.40 },
    { text: '  ECS Entity Manager ..................................... [OK]',        type: 'success', delay: 0.35 },
    { text: '  Event System ............................................ [OK]',       type: 'success', delay: 0.35 },
    { text: '',                                                                       type: 'info',    delay: 0.00 },
    { text: 'Loading Resources...',                                                   type: 'info',    delay: 0.40 },
    { text: '  3D Model ................................................ [OK]',       type: 'success', delay: 0.35 },
    { text: '  Texture ................................................. [OK]',       type: 'success', delay: 0.35 },
    { text: '  Audio ................................................... [OK]',       type: 'success', delay: 0.35 },
    { text: '',                                                                       type: 'info',    delay: 0.00 },
    { text: 'Placing Enemies...',                                                     type: 'info',    delay: 0.40 },
    { text: '  Enemy 1 ................................................ [PLACE]',     type: 'info',    delay: 0.25 },
    { text: '  Enemy 2 ................................................ [PLACE]',     type: 'info',    delay: 0.25 },
    { text: '  Boss ................................................... [PLACE]',     type: 'warning', delay: 0.30 },
    { text: '',                                                                       type: 'info',    delay: 0.00 },
    { text: 'Dungeon Initialization Complete. Starting Game...',                      type: 'success', delay: 0.80 },
];

const LoadingLogic = (function () {
    let messageQueue = [];
    let visibleCount = 0;
    let elapsedTime = 0;
    let lineTimestamps = [];
    let isFinished = false;
    let onLineReadyCallback = null;
    let onLoadingCompleteCallback = null;

    function initialize() {
        // メッセージキューのセットアップ
        messageQueue = JSON.parse(JSON.stringify(LOADING_MESSAGES));
        visibleCount = 0;
        elapsedTime = 0;
        isFinished = false;

        // 各行の表示タイミング（累積時間）を事前計算
        lineTimestamps = [];
        let acc = 0;
        for (let i = 0; i < messageQueue.length; ++i) {
            acc += messageQueue[i].delay;
            lineTimestamps.push(acc);
        }
    }

    function update(deltaTime) {
        if (isFinished) return;

        elapsedTime += deltaTime;

        // 表示する行数を更新
        while (visibleCount < messageQueue.length &&
            elapsedTime >= lineTimestamps[visibleCount]) {
            if (onLineReadyCallback) {
                const msg = messageQueue[visibleCount];
                onLineReadyCallback(msg.text, msg.type);
            }
            visibleCount++;
        }

        // すべての行が表示されたら完了
        if (visibleCount >= messageQueue.length && !isFinished) {
            isFinished = true;
            if (onLoadingCompleteCallback) {
                onLoadingCompleteCallback();
            }
        }
    }

    function onMessageFromGame(data) {
        if (data.type === 'startLoading') {
            initialize();
        }
    }

    function onLineReady(callback) {
        onLineReadyCallback = callback;
    }

    function onLoadingComplete(callback) {
        onLoadingCompleteCallback = callback;
    }

    function getElapsedTime() {
        return elapsedTime;
    }

    function isLoadingFinished() {
        return isFinished;
    }

    // 初期化を実行
    initialize();

    // ゲームループの代わりに、定期的にupdateを呼び出す必要がある
    // (C++側で呼び出されるか、JavaScriptで setInterval/requestAnimationFrame を使用)
    setInterval(function () {
        update(0.016); // 約60FPS
    }, 16);

    return {
        update: update,
        onMessageFromGame: onMessageFromGame,
        onLineReady: onLineReady,
        onLoadingComplete: onLoadingComplete,
        getElapsedTime: getElapsedTime,
        isLoadingFinished: isLoadingFinished
    };
}());
