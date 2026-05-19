'use strict';

/**
 * ウィンドウサイズに応じて CSS 変数を動的に更新
 * Debug: 1280x720, Release: フルスクリーン対応
 */
function updateResponsiveVariables() {
    const width = window.innerWidth;
    const height = window.innerHeight;
    const rootElement = document.documentElement;

    // 基準値（Debug 1280x720 時）
    const baseWidth = 1280;
    const baseHeight = 720;
    const baseTaskbarHeight = 48;
    const baseIconSize = 32;

    // 画面のスケール比（小さい方の比率を採用）
    const scaleX = width / baseWidth;
    const scaleY = height / baseHeight;
    const scale = Math.min(scaleX, scaleY);

    // CSS 変数をスケール値で更新
    const taskbarHeight = Math.max(48, Math.round(baseTaskbarHeight * scale));
    const iconSize = Math.max(32, Math.round(baseIconSize * scale));
    const appPadding = Math.max(4, Math.round(6 * scale));
    const tooltipFontSize = Math.max(10, Math.round(11 * scale));
    const clockFontSize = Math.max(10, Math.round(11 * scale));
    const startBtnFontSize = Math.max(12, Math.round(13 * scale));

    rootElement.style.setProperty('--taskbar-height', taskbarHeight + 'px');
    rootElement.style.setProperty('--icon-size', iconSize + 'px');
    rootElement.style.setProperty('--app-padding', appPadding + 'px');
    rootElement.style.setProperty('--tooltip-font-size', tooltipFontSize + 'px');
    rootElement.style.setProperty('--clock-font-size', clockFontSize + 'px');
    rootElement.style.setProperty('--start-btn-font-size', startBtnFontSize + 'px');

    // スケール値が小さい場合はスペーシングも調整
    if (scale < 0.9) {
        rootElement.style.setProperty('--taskbar-gap', Math.round(6 * scale) + 'px');
        rootElement.style.setProperty('--taskbar-right-gap', Math.round(8 * scale) + 'px');
    }
}

// 初期化 + リサイズ監視
updateResponsiveVariables();
window.addEventListener('resize', updateResponsiveVariables);

const winStates = { job: true, file: true, param: true, diff: true };

function toggleWindow(name) {
    sendToGame({ type: 'toggleWindow', window: name });
}

function startGame() {
    sendToGame({ type: 'startGame' });
}

function setWindowVisible(name, visible) {
    winStates[name] = visible;
    const appElement = document.getElementById('app-' + name);
    if (!appElement) return;
    if (visible) {
        appElement.classList.add('active');
        appElement.classList.remove('hidden-win');
    } else {
        appElement.classList.remove('active');
        appElement.classList.add('hidden-win');
    }
}

function onMessageFromGame(data) {
    if (data.type === 'windowStateChanged') {
        setWindowVisible(data.window, data.visible);
    }
}

function updateClock() {
    const now = new Date();
    const h  = String(now.getHours()).padStart(2, '0');
    const m  = String(now.getMinutes()).padStart(2, '0');
    const y  = now.getFullYear();
    const mo = String(now.getMonth() + 1).padStart(2, '0');
    const d  = String(now.getDate()).padStart(2, '0');
    document.getElementById('clock').innerHTML = h + ':' + m + '<br>' + y + '/' + mo + '/' + d;
}

updateClock();
setInterval(updateClock, 10000);
