'use strict';

/**
 * デスクトップのビュー層
 * CSS変数・時計・ウィンドウ表示制御・Matrix Rain を担う
 */
const DesktopView = (function () {

    // ── レスポンシブ CSS 変数 ─────────────────────────────────────

    /**
     * @brief ウィンドウサイズに応じて CSS 変数を動的に更新する
     * Debug: 1280x720, Release: フルスクリーン対応
     */
    function updateResponsiveVariables() {
        const width       = window.innerWidth;
        const height      = window.innerHeight;
        const rootElement = document.documentElement;

        // 基準値（Debug 1280x720 時）
        const baseWidth         = 1280;
        const baseHeight        = 720;
        const baseTaskbarHeight = 48;
        const baseIconSize      = 32;

        // 画面のスケール比（小さい方の比率を採用）
        const scale = Math.min(width / baseWidth, height / baseHeight);

        const taskbarHeight    = Math.max(48, Math.round(baseTaskbarHeight * scale));
        const iconSize         = Math.max(32, Math.round(baseIconSize * scale));
        const appPadding       = Math.max(4,  Math.round(6  * scale));
        const tooltipFontSize  = Math.max(10, Math.round(11 * scale));
        const clockFontSize    = Math.max(10, Math.round(11 * scale));
        const startBtnFontSize = Math.max(12, Math.round(13 * scale));

        rootElement.style.setProperty('--taskbar-height',       taskbarHeight    + 'px');
        rootElement.style.setProperty('--icon-size',            iconSize         + 'px');
        rootElement.style.setProperty('--app-padding',          appPadding       + 'px');
        rootElement.style.setProperty('--tooltip-font-size',    tooltipFontSize  + 'px');
        rootElement.style.setProperty('--clock-font-size',      clockFontSize    + 'px');
        rootElement.style.setProperty('--start-btn-font-size',  startBtnFontSize + 'px');

        if (scale < 0.9) {
            rootElement.style.setProperty('--taskbar-gap',       Math.round(6 * scale) + 'px');
            rootElement.style.setProperty('--taskbar-right-gap', Math.round(8 * scale) + 'px');
        }
    }

    // ── ウィンドウ表示状態 ────────────────────────────────────────

    /**
     * @brief タスクバーアイコンの active / hidden-win クラスを更新する
     * @param {string}  name    ウィンドウ識別子
     * @param {boolean} visible 表示するか否か
     */
    function setWindowVisible(name, visible) {
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

    // ── 時計 ──────────────────────────────────────────────────────

    /**
     * @brief 時計表示を現在時刻で更新する
     */
    function updateClock() {
        const now = new Date();
        const h   = String(now.getHours()).padStart(2, '0');
        const m   = String(now.getMinutes()).padStart(2, '0');
        const y   = now.getFullYear();
        const mo  = String(now.getMonth() + 1).padStart(2, '0');
        const d   = String(now.getDate()).padStart(2, '0');
        document.getElementById('clock').innerHTML = h + ':' + m + '<br>' + y + '/' + mo + '/' + d;
    }

    // ── Matrix Rain ───────────────────────────────────────────────

    /**
     * @brief Matrix Rain アニメーションを初期化・開始する
     */
    function initMatrixRain() {
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
            // C
            '#include <stdio.h>',
            '#include <stdlib.h>',
            'malloc(sizeof(struct Node))',
            'free(ptr);',
            'printf("%d\\n", val);',
            'int arr[256] = {0};',
            'sizeof(int) * n',
            'FILE *fp = fopen("data", "r");',
            'memset(buf, 0, sizeof(buf));',
            'typedef struct { int x, y; } Vec2;',
            // C# (Unity)
            'void Start() { }',
            'void Update() { }',
            'void Awake() { }',
            'GetComponent<Rigidbody>()',
            'Instantiate(prefab, pos, rot)',
            '[SerializeField] int hp;',
            'StartCoroutine(Fade());',
            'yield return new WaitForSeconds(1f);',
            'transform.position += Vector3.up;',
            'Physics.Raycast(ray, out hit)',
            'void OnCollisionEnter(Collision col)',
            'SceneManager.LoadScene("Game")',
            'public class Player : MonoBehaviour',
            // JavaScript
            'const canvas = document.getElementById("c");',
            'ctx.clearRect(0, 0, w, h);',
            'requestAnimationFrame(loop);',
            'addEventListener("keydown", onKey);',
            'const { x, y } = player;',
            'Array.from({length: n}, () => 0)',
            'Math.floor(Math.random() * 100)',
            'setTimeout(() => resolve(), 500)',
            // Python
            'def __init__(self):',
            'if __name__ == "__main__":',
            'import numpy as np',
            'for i, v in enumerate(lst):',
            'lambda x: x * x',
        ];

        // 単語プール: PROG_LINES をスペース区切りで分割し 3 文字以上のトークンのみ収集
        const WORD_POOL = [];
        PROG_LINES.forEach(function (line) {
            line.split(/\s+/).forEach(function (t) {
                if (t.length >= 3) WORD_POOL.push(t);
            });
        });

        const COL_W = 110;  // カラム幅 (px)
        const ROW_H = 18;   // 1 トークンあたりの行高さ (px)
        let cols, drops;

        function randomWords(count) {
            return Array.from({ length: count }, function () {
                return WORD_POOL[Math.floor(Math.random() * WORD_POOL.length)];
            });
        }

        function initDrops() {
            cols = Math.floor(canvas.width / COL_W);
            drops = Array.from({ length: cols }, function () {
                return {
                    y:     -Math.random() * canvas.height,
                    spd:   0.35 + Math.random() * 0.55,
                    words: randomWords(30),
                    alpha: 0.50 + Math.random() * 0.35,
                };
            });
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
            const trailLen = 12;

            for (let i = 0; i < drops.length; i++) {
                const d      = drops[i];
                const x      = i * COL_W + 4;
                const visIdx = Math.floor(d.y / ROW_H);

                for (let wi = Math.max(0, visIdx - trailLen); wi <= visIdx && wi < d.words.length; wi++) {
                    const isTip = wi === visIdx;
                    const fade  = isTip ? 1.0 : (1 - (visIdx - wi) / trailLen) * 0.6;
                    ctx.fillStyle = isTip
                        ? `rgba(150, 210, 255, ${d.alpha})`
                        : `rgba(30, 80, 180, ${d.alpha * fade * 0.65})`;
                    ctx.fillText(d.words[wi], x, wi * ROW_H + 16);
                }

                d.y += d.spd;
                if (d.y > (d.words.length + 14) * ROW_H) {
                    d.y     = -Math.random() * canvas.height * 0.5;
                    d.words = randomWords(30);
                    d.spd   = 0.35 + Math.random() * 0.55;
                    d.alpha = 0.45 + Math.random() * 0.40;
                }
            }
        }
    }

    // ── 初期化 ────────────────────────────────────────────────────

    /**
     * @brief ビュー全体を初期化する（desktop-logic.js より後に呼ばれること）
     */
    function initialize() {
        updateResponsiveVariables();
        window.addEventListener('resize', updateResponsiveVariables);

        updateClock();
        setInterval(updateClock, 10000);

        initMatrixRain();

        DesktopLogic.onWindowChange(function (name, visible) {
            setWindowVisible(name, visible);
        });
    }

    return { initialize };
}());

DesktopView.initialize();
