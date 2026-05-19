'use strict';

const ResultView = (function () {
    function renderWin(data) {
        const rank = ResultLogic.calcRank(data);
        const files = (data.usedFiles || []).map(function (f) {
            const name = f.replace(/.*[\\/]/, '');
            return '<span class="result-file-badge">' + ResultLogic.escapeHtml(name) + '</span>';
        }).join('');
        const filesHtml = files
            ? '<div class="result-file-badges">' + files + '</div>'
            : '<span class="result-stat-value">—</span>';

        return '<div class="result-content good-end">'
            + '<div class="result-message-row">'
            + '<div class="result-message-icon checkmark">✅</div>'
            + '<div class="result-message-body">'
            + '<div class="result-message-main">MacBook を無事に停止させました。</div>'
            + '</div></div>'
            + '<div class="result-rank-row">'
            + '<span class="result-rank-label">評価</span>'
            + '<span class="result-rank-value rank-' + rank.toLowerCase() + '">' + rank + '</span>'
            + '</div>'
            + '<div class="result-divider"></div>'
            + '<div class="result-stats">'
            + '<div class="result-stat-row"><span class="result-stat-label">撃破数</span><span class="result-stat-value">' + (data.killCount || 0) + '</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">クリア時間</span><span class="result-stat-value">' + ResultLogic.formatTime(data.elapsedTime) + '</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">被ダメージ</span><span class="result-stat-value">' + Math.floor(data.totalDamageTaken || 0) + '</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">使用ファイル</span>' + filesHtml + '</div>'
            + '</div>'
            + '<div class="result-divider"></div>'
            + '<div class="result-btn-row">'
            + '<button class="result-btn" data-action="retry">もう一度</button>'
            + '<button class="result-btn" data-action="title">タイトルへ</button>'
            + '</div>'
            + '</div>';
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
            + '<div class="result-content bad-end" id="lose-dialog" style="display:none">'
            + '<div class="result-message-row">'
            + '<div class="result-message-icon error">⛔</div>'
            + '<div class="result-message-body">'
            + '<div class="result-message-main">Windows が致命的なエラーで停止しました。</div>'
            + '<div class="result-message-sub">敵の攻撃により、プロセスを強制終了します。</div>'
            + '</div></div>'
            + '<div class="result-divider"></div>'
            + '<div class="result-stats">'
            + '<div class="result-stat-row"><span class="result-stat-label">終了コード</span><span class="result-stat-value">0x000000_DEFEATED</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">発生場所</span><span class="result-stat-value">MacBook.exe</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">生存時間</span><span class="result-stat-value">' + ResultLogic.formatTime(data.elapsedTime) + '</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">被ダメージ</span><span class="result-stat-value">' + Math.floor(data.totalDamageTaken || 0) + '</span></div>'
            + '</div>'
            + '<div class="result-divider"></div>'
            + '<div class="result-btn-row">'
            + '<button class="result-btn" data-action="retry">再試行</button>'
            + '<button class="result-btn" data-action="title">タイトルへ</button>'
            + '</div>'
            + '</div>';
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
            if (dialogElement) {
                dialogElement.style.display = 'flex';
                dialogElement.style.flexDirection = 'column';
            }
            setupButtons();
        }, 600);
    }

    function render(data) {
        const rootElement = document.getElementById('result-root');
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

(function () {
    ResultLogic.onResultData(function (data) {
        ResultView.render(data);
    });

    ResultLogic.requestResult();
}());
