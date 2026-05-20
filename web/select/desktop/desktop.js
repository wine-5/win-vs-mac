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

// ── Matrix Rain ──────────────────────────────────────────────────
(function () {
    const canvas = document.getElementById('matrix-bg');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    const PROG_LINES = [
        // C++
        'for(int i = 0; i < n; i++)',
        'if(ptr == nullptr) return;',
        'std::vector<int> vec;',
        '#include <iostream>',
        'template<typename T>',
        'virtual void update() = 0;',
        'auto& [key, val] : map',
        'std::make_unique<T>()',
        'reinterpret_cast<void*>(p)',
        'constexpr int MAX = 256;',
        'delete[] ptr; ptr = nullptr;',
        '[[nodiscard]] bool isValid()',
        '#pragma once',
        'assert(value != nullptr);',
        'throw std::runtime_error(msg)',
        '// TODO: fix this',
        '/* FIXME: memory leak */',
        'struct Node { int val; Node* next; };',
        'std::unordered_map<K, V>',
        'std::priority_queue<int>',
        // Python
        'def __init__(self):',
        'if __name__ == "__main__":',
        'import numpy as np',
        'for i, v in enumerate(lst):',
        'lambda x: x * x',
        // Algorithms
        'O(n log n)', 'O(1) space',
        'binary search', 'merge sort',
        'BFS', 'DFS', 'Dijkstra',
        'dp[i] = dp[i-1] + dp[i-2]',
        'heap.push(node)',
        // Git
        'git commit -m "feat: add"',
        'git push origin main',
        'git rebase -i HEAD~3',
        'fatal: not a git repository',
        'HEAD is now at a3f9b12',
        // Errors / Build
        'error: use of undeclared identifier',
        'warning: unused variable [-Wunused]',
        'Segmentation fault (core dumped)',
        'undefined reference to `main`',
        'gcc -O2 -Wall -o out main.c',
        'make clean && make all',
        'cmake .. -DCMAKE_BUILD_TYPE=Release',
    ];

    const CHAR_H = 15;
    let cols, drops;

    function initDrops() {
        cols = Math.floor(canvas.width / 14);
        drops = Array.from({ length: cols }, () => ({
            y:     -Math.random() * canvas.height,
            spd:   0.35 + Math.random() * 0.65,
            txt:   PROG_LINES[Math.floor(Math.random() * PROG_LINES.length)],
            alpha: 0.15 + Math.random() * 0.22,
        }));
    }

    function resize() {
        canvas.width  = window.innerWidth;
        canvas.height = window.innerHeight;
        initDrops();
    }
    window.addEventListener('resize', resize);
    // DOM レイアウト確定後にサイズを取得するため 1 フレーム待つ
    requestAnimationFrame(function () { resize(); draw(); });

    function draw() {
        requestAnimationFrame(draw);
        // 完全にクリアして文字だけを描画（背景画像を透過させる）
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        ctx.font = '11px Consolas, monospace';
        const colW = canvas.width / cols;

        for (let i = 0; i < drops.length; i++) {
            const d = drops[i];
            const visLen = Math.floor(d.y / CHAR_H);
            const trailLen = 20;

            for (let ci = Math.max(0, visLen - trailLen); ci <= visLen && ci < d.txt.length; ci++) {
                const fade = ci === visLen ? 1 : (1 - (visLen - ci) / trailLen) * 0.55;
                ctx.fillStyle = ci === visLen
                    ? `rgba(150, 210, 255, ${d.alpha * fade})`
                    : `rgba(30, 80, 180, ${d.alpha * fade * 0.6})`;
                ctx.fillText(d.txt[ci], i * colW, ci * CHAR_H + 14);
            }

            d.y += d.spd;
            if (d.y > (d.txt.length + 22) * CHAR_H) {
                d.y     = -Math.random() * canvas.height;
                d.txt   = PROG_LINES[Math.floor(Math.random() * PROG_LINES.length)];
                d.spd   = 0.35 + Math.random() * 0.65;
                d.alpha = 0.12 + Math.random() * 0.20;
            }
        }
    }
    // draw() は resize() の requestAnimationFrame コールバック内で開始する
}());
