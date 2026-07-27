'use strict';

const ResultView = (function () {
    // ランクごとの見出し。評価軸がタイムだけになったので、
    // 文言も「速さ」の話に統一している
    const RANK_CAPTIONS = {
        S: 'PERFECT RUN — 理論値に迫る制圧速度',
        A: 'FAST — 無駄のない立ち回り',
        B: 'STABLE — 安定した処理速度',
        C: 'SLOW — まだ詰められます',
        D: 'TIMEOUT — 大幅な短縮の余地あり'
    };

    /**
     * ヘッダー（達成表示と難易度バッジ）を組み立てる
     * @param {object} data リザルトデータ
     * @returns {string} HTML文字列
     */
    function buildHeader(data) {
        const difficulty = data.difficulty === 'HARD' ? 'HARD' : 'NORMAL';
        return '<header class="result-header">'
            + '<div class="result-eyebrow">'
            + '<span class="result-eyebrow-dot"></span>MISSION COMPLETE'
            + '</div>'
            + '<span class="result-difficulty-badge">' + difficulty + '</span>'
            + '</header>'
            + '<h1 class="result-title">MacBook を無事に停止させました</h1>';
    }

    /**
     * ランクの主役表示を組み立てる
     * @param {string} rank ランク文字
     * @returns {string} HTML文字列
     */
    function buildRankHero(rank) {
        const lower = rank.toLowerCase();
        return '<section class="rank-hero rank-' + lower + '" id="rank-hero">'
            + '<div class="rank-shockwave"></div>'
            + '<div class="rank-glow"></div>'
            + '<div class="rank-letter" id="rank-letter" data-rank="' + rank + '">' + rank + '</div>'
            + '<div class="rank-caption">' + ResultLogic.escapeHtml(RANK_CAPTIONS[rank] || '') + '</div>'
            + '</section>';
    }

    /**
     * クリアタイムと次ランクまでの目安を組み立てる
     * @param {object} data リザルトデータ
     * @returns {string} HTML文字列
     */
    function buildTimePanel(data) {
        return '<section class="time-panel">'
            + '<div class="section-label">CLEAR TIME</div>'
            + '<div class="time-value" id="time-value">'
            + ResultLogic.formatClock(data.elapsedTime) + '</div>'
            + '</section>';
    }

    /**
     * 補助スタットのタイル群を組み立てる
     * @param {object} data リザルトデータ
     * @returns {string} HTML文字列
     */
    function buildStatTiles(data) {
        const tiles = [
            { label: '撃破数', value: String(data.killCount || 0), unit: '体' },
            { label: '被ダメージ', value: String(Math.floor(data.totalDamageTaken || 0)), unit: '' },
            { label: '使用ファイル', value: String((data.usedFiles || []).length), unit: '個' }
        ];

        return '<section class="stat-tiles">'
            + tiles.map(function (tile) {
                return '<div class="stat-tile">'
                    + '<div class="stat-tile-label">' + tile.label + '</div>'
                    + '<div class="stat-tile-value">' + ResultLogic.escapeHtml(tile.value)
                    + (tile.unit ? '<span class="stat-tile-unit">' + tile.unit + '</span>' : '')
                    + '</div></div>';
            }).join('')
            + '</section>';
    }

    /**
     * 装備していたファイルのバッジ列を組み立てる
     * @param {object} data リザルトデータ
     * @returns {string} HTML文字列
     */
    function buildFileSection(data) {
        const files = data.usedFiles || [];
        if (files.length === 0) return '';

        const badges = files.map(function (f) {
            const name = f.replace(/.*[\\/]/, '');
            const ext = name.indexOf('.') >= 0 ? name.split('.').pop().toLowerCase() : '';
            return '<span class="result-file-badge">'
                + '<span class="result-file-ext">.' + ResultLogic.escapeHtml(ext) + '</span>'
                + ResultLogic.escapeHtml(name)
                + '</span>';
        }).join('');

        return '<section class="result-files">'
            + '<div class="section-label">EQUIPPED FILES</div>'
            + '<div class="result-file-badges">' + badges + '</div>'
            + '</section>';
    }

    /**
     * 操作ボタン行を組み立てる
     * @param {string} retryLabel リトライ側の文言
     * @returns {string} HTML文字列
     */
    function buildButtons(retryLabel) {
        return '<div class="result-btn-row">'
            + '<button class="result-btn primary" data-action="retry">' + retryLabel + '</button>'
            + '<button class="result-btn" data-action="title">タイトルへ</button>'
            + '</div>';
    }

    function renderWin(data) {
        const rank = ResultLogic.calcRank(data);

        return '<div class="result-stage good-end">'
            + '<div class="result-card">'
            + buildHeader(data)
            + buildRankHero(rank)
            + buildTimePanel(data)
            + buildStatTiles(data)
            + buildFileSection(data)
            + buildButtons('もう一度')
            + '</div></div>';
    }

    function renderLose(data) {
        return '<div class="bsod-overlay" id="bsod-overlay">'
            + '<div class="bsod-content">'
            + '<div class="bsod-face">:(</div>'
            + '<div class="bsod-title">ご使用の PC で問題が発生したため、再起動する必要があります。<br>エラーの収集が完了したら、再起動します。</div>'
            + '<div><div class="bsod-bar-track"><div class="bsod-bar-fill" id="bsod-fill"></div></div>'
            + '<div class="bsod-progress-text"><span id="bsod-pct">0</span>% 完了</div></div>'
            + '<div class="bsod-stop-code">詳細については、以下のエラーを検索してください:<br>停止コード: <strong>PLAYER_DEFEATED</strong></div>'
            + '</div></div>'
            + '<div class="result-stage bad-end" id="lose-dialog" style="display:none">'
            + '<div class="result-card">'
            + '<header class="result-header">'
            + '<div class="result-eyebrow error">'
            + '<span class="result-eyebrow-dot"></span>PROCESS TERMINATED'
            + '</div>'
            + '<span class="result-difficulty-badge">' + (data.difficulty === 'HARD' ? 'HARD' : 'NORMAL') + '</span>'
            + '</header>'
            + '<h1 class="result-title">Windows が致命的なエラーで停止しました</h1>'
            + '<p class="result-subtitle">敵の攻撃により、プロセスを強制終了します。</p>'
            + '<section class="stat-tiles">'
            + '<div class="stat-tile"><div class="stat-tile-label">生存時間</div>'
            + '<div class="stat-tile-value">' + ResultLogic.formatClock(data.elapsedTime) + '</div></div>'
            + '<div class="stat-tile"><div class="stat-tile-label">撃破数</div>'
            + '<div class="stat-tile-value">' + (data.killCount || 0) + '<span class="stat-tile-unit">体</span></div></div>'
            + '<div class="stat-tile"><div class="stat-tile-label">被ダメージ</div>'
            + '<div class="stat-tile-value">' + Math.floor(data.totalDamageTaken || 0) + '</div></div>'
            + '</section>'
            + '<section class="crash-detail">'
            + '<div class="crash-row"><span>終了コード</span><span>0x000000_DEFEATED</span></div>'
            + '<div class="crash-row"><span>発生場所</span><span>MacBook.exe</span></div>'
            + '</section>'
            + buildButtons('再試行')
            + '</div></div>';
    }

    function startBsodAnimation() {
        const fillElement = document.getElementById('bsod-fill');
        const pctElement = document.getElementById('bsod-pct');
        if (!fillElement || !pctElement) return;

        let progress = 0;
        const timer = setInterval(function () {
            progress += 1;
            fillElement.style.width = progress + '%';
            pctElement.textContent = String(progress);
            if (progress >= 100) {
                clearInterval(timer);
                setTimeout(revealLoseDialog, 400);
            }
        }, 40);
    }

    function revealLoseDialog() {
        const overlayElement = document.getElementById('bsod-overlay');
        const dialogElement = document.getElementById('lose-dialog');
        if (!overlayElement) return;

        overlayElement.classList.add('fade-out');
        setTimeout(function () {
            overlayElement.style.display = 'none';
            if (dialogElement)
                dialogElement.style.display = 'flex';
            setupButtons();
        }, 600);
    }

    function render(data) {
        const rootElement = document.getElementById('result-root');
        // HARD は配色そのものを警告色へ寄せる（common.css の body.hard）
        document.body.classList.toggle('hard', data.difficulty === 'HARD');

        if (data.isVictory) {
            rootElement.innerHTML = renderWin(data);
            setupButtons();
        } else {
            rootElement.innerHTML = renderLose(data);
            startBsodAnimation();
        }
    }

    function setupButtons() {
        document.querySelectorAll('[data-action]').forEach(function (btn) {
            btn.addEventListener('click', function () {
                sendToGame({ type: btn.dataset.action });
            });
        });
    }

    return {
        render: render,
        renderWin: renderWin,
        renderLose: renderLose,
        setupButtons: setupButtons,
        startBsodAnimation: startBsodAnimation,
        revealLoseDialog: revealLoseDialog
    };
}());

// グローバルスコープに onMessageFromGame を定義（messaging.js から呼ばれる）
function onMessageFromGame(data) {
    ResultLogic.onMessageFromGame(data);
}

(function () {
    ResultLogic.onResultData(function (data) {
        ResultView.render(data);
    });

    ResultLogic.requestResult();
}());
