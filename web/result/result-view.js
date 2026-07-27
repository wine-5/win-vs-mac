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

        // 上位ランクほど演出を足す。下位で派手に光ると「褒められている」感が出てしまう
        const shine = (rank === 'S' || rank === 'A') ? '<div class="rank-shine"></div>' : '';
        const sparks = rank === 'S' ? buildSparks() : '';

        return '<section class="rank-hero rank-' + lower + '" id="rank-hero">'
            + '<div class="rank-shockwave"></div>'
            + '<div class="rank-shockwave delayed"></div>'
            + '<div class="rank-glow"></div>'
            + sparks
            + '<div class="rank-letter" id="rank-letter" data-rank="' + rank + '">' + rank + '</div>'
            + shine
            + '<div class="rank-caption">' + ResultLogic.escapeHtml(RANK_CAPTIONS[rank] || '') + '</div>'
            + '</section>';
    }

    /**
     * Sランクで着弾に合わせて飛ばす火花を組み立てる
     * @returns {string} HTML文字列
     */
    function buildSparks() {
        const COUNT = 10;
        let html = '<div class="rank-sparks">';
        for (let i = 0; i < COUNT; i++) {
            // 放射状に等間隔で飛ばす。距離だけ交互に変えて機械的な並びを崩す
            const angle = (360 / COUNT) * i;
            const distance = 68 + (i % 3) * 22;
            html += '<span class="rank-spark" style="'
                + '--spark-angle:' + angle + 'deg;'
                + '--spark-distance:' + distance + 'px;'
                + 'animation-delay:' + (0.5 + (i % 4) * 0.03).toFixed(2) + 's"></span>';
        }
        return html + '</div>';
    }

    /**
     * クリアタイムと次ランクまでの目安を組み立てる
     * @param {object} data リザルトデータ
     * @returns {string} HTML文字列
     */
    function buildTimePanel(data) {
        const goal = ResultLogic.calcNextRankGoal(data);
        const elapsed = data.elapsedTime || 0;

        let goalHtml = '';
        if (goal) {
            // 上のランクの制限時間に対して、今のタイムがどこまで収まっているかを見せる。
            // 100%を超えた分は溢れて見えないので上限で止める
            const budget = Math.max(0, elapsed - goal.remainSeconds);
            const ratio = elapsed > 0 ? Math.min(1, budget / elapsed) : 0;
            goalHtml = '<div class="next-rank">'
                + '<div class="next-rank-track">'
                + '<div class="next-rank-fill" style="width:' + (ratio * 100).toFixed(1) + '%"></div>'
                + '</div>'
                + '<div class="next-rank-text">あと <strong>' + Math.ceil(goal.remainSeconds) + '</strong> 秒 縮めると '
                + '<span class="next-rank-letter rank-' + goal.rank.toLowerCase() + '">' + goal.rank + '</span> ランク'
                + '</div></div>';
        } else {
            goalHtml = '<div class="next-rank">'
                + '<div class="next-rank-text max">最高ランク到達 — これ以上はありません</div>'
                + '</div>';
        }

        return '<section class="time-panel">'
            + '<div class="section-label">CLEAR TIME</div>'
            + '<div class="time-value" id="time-value">'
            + ResultLogic.formatClock(elapsed) + '</div>'
            + goalHtml
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
            + '<div class="bsod-title">このPCは MacBook.exe に掌握されました。<br>エージェントを回収したら、システムを巻き戻します。</div>'
            + '<div><div class="bsod-bar-track"><div class="bsod-bar-fill" id="bsod-fill"></div></div>'
            + '<div class="bsod-progress-text"><span id="bsod-pct">0</span>% 回収しました</div></div>'
            + '<div class="bsod-stop-code">対処方法については、管理者権限で以下を照会してください:<br>停止コード: <strong>AGENT_TERMINATED_BY_MACBOOK</strong></div>'
            + '</div></div>'
            + '<div class="result-stage bad-end" id="lose-dialog" style="display:none">'
            + '<div class="result-card">'
            + '<header class="result-header">'
            + '<div class="result-eyebrow error">'
            + '<span class="result-eyebrow-dot"></span>PROCESS TERMINATED'
            + '</div>'
            + '<span class="result-difficulty-badge">' + (data.difficulty === 'HARD' ? 'HARD' : 'NORMAL') + '</span>'
            + '</header>'
            + '<h1 class="result-title">この PC は MacBook.exe に明け渡されました</h1>'
            + '<p class="result-subtitle">エージェントが応答を停止したため、システムから切り離しました。</p>'
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
            + '<div class="crash-row"><span>障害モジュール</span><span>MacBook.exe</span></div>'
            + '<div class="crash-row"><span>失われたプロセス</span><span>SecurityAgent.exe</span></div>'
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

    // ランク文字が着弾する瞬間（CSSの rankStamp の遅延0.25s＋落下ぶんに合わせている）
    const RANK_IMPACT_MS = 520;

    // 着弾に鳴らすSE。Sだけ会心音にして特別扱いする
    const RANK_IMPACT_SE = { S: 'Critical', A: 'HitChargedWindow' };

    /**
     * ランクの着弾に合わせて音と余韻の光を入れる
     * @param {string} rank ランク文字
     */
    function startRankSequence(rank) {
        setTimeout(function () {
            sendToGame({ type: 'uiSound', se: RANK_IMPACT_SE[rank] || 'UiFileSelect' });

            // 着弾しきってから呼吸する光へ移す。落下中から光らせると衝撃が弱まる
            const hero = document.getElementById('rank-hero');
            if (hero) hero.classList.add('settled');
        }, RANK_IMPACT_MS);
    }

    /**
     * クリアタイムを 00:00 から実測値まで数え上げる
     * @param {number} seconds 実際のクリアタイム（秒）
     */
    function startTimeCountUp(seconds) {
        const element = document.getElementById('time-value');
        if (!element || seconds <= 0) return;

        const DURATION_MS = 900;
        const startedAt = performance.now();

        function step(now) {
            const t = Math.min(1, (now - startedAt) / DURATION_MS);
            // 終わり際をゆっくりにして、止まる瞬間に目が行くようにする
            const eased = 1 - Math.pow(1 - t, 3);
            element.textContent = ResultLogic.formatClock(seconds * eased);
            if (t < 1) requestAnimationFrame(step);
        }

        // タイムパネル自体のフェードイン（0.65s）が終わる頃から動かす
        setTimeout(function () { requestAnimationFrame(step); }, 700);
    }

    function render(data) {
        const rootElement = document.getElementById('result-root');
        // HARD は配色そのものを警告色へ寄せる（common.css の body.hard）
        document.body.classList.toggle('hard', data.difficulty === 'HARD');

        if (data.isVictory) {
            rootElement.innerHTML = renderWin(data);
            setupButtons();
            startRankSequence(ResultLogic.calcRank(data));
            startTimeCountUp(data.elapsedTime || 0);
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
