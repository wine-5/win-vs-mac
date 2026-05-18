(function () {
    'use strict';

    // ---- ヘルパー関数 ----

    function formatTime(seconds) {
        var m = Math.floor(seconds / 60);
        var s = Math.floor(seconds % 60);
        return m + ':' + String(s).padStart(2, '0');
    }

    function escapeHtml(str) {
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;');
    }

    // 時間と被ダメージからランクを算出
    function calcRank(data) {
        var dmg = data.totalDamageTaken || 0;
        var t   = data.elapsedTime || 0;
        if (dmg === 0 && t <  30) return 'S';
        if (dmg === 0 && t < 120) return 'A';
        if (dmg <  50)            return 'B';
        if (dmg < 150)            return 'C';
        return 'D';
    }

    // ---- 勝利ダイアログ ----

    function renderWin(data) {
        var rank  = calcRank(data);
        var files = (data.usedFiles || []).map(function (f) {
            var name = f.replace(/.*[\\/]/, '');
            return '<span class="result-file-badge">' + escapeHtml(name) + '</span>';
        }).join('');
        var filesHtml = files
            ? '<div class="result-file-badges">' + files + '</div>'
            : '<span class="result-stat-value">—</span>';

        return '<div class="result-content good-end">'
            + '<div class="result-message-row">'
            + '<div class="result-message-icon checkmark">✅</div>'
            + '<div class="result-message-body">'
            + '<div class="result-message-main">MacBook を無事に停止させました。</div>'
            + '</div></div>'
            // 評価ランク
            + '<div class="result-rank-row">'
            + '<span class="result-rank-label">評価</span>'
            + '<span class="result-rank-value rank-' + rank.toLowerCase() + '">' + rank + '</span>'
            + '</div>'
            + '<div class="result-divider"></div>'
            // スタット一覧
            + '<div class="result-stats">'
            + '<div class="result-stat-row"><span class="result-stat-label">撃破数</span><span class="result-stat-value">' + (data.killCount || 0) + '</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">クリア時間</span><span class="result-stat-value">' + formatTime(data.elapsedTime) + '</span></div>'
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

    // ---- 敗北画面（BSOD → エラーダイアログ） ----

    function renderLose(data) {
        return '<div class="bsod-overlay" id="bsod-overlay">'
            + '<div class="bsod-content">'
            + '<div class="bsod-face">:(</div>'
            + '<div class="bsod-title">ご使用の PC で問題が発生したため、再起動する必要があります。<br>エラーの収集が完了したら、再起動します。</div>'
            + '<div><div class="bsod-bar-track"><div class="bsod-bar-fill" id="bsod-fill"></div></div>'
            + '<div class="bsod-progress-text"><span id="bsod-pct">0</span>% 完了</div></div>'
            + '<div class="bsod-stop-code">詳細については、以下のエラーを検索してください:<br>停止コード: <strong>PLAYER_DEFEATED</strong></div>'
            + '</div></div>'
            // BSOD フェードアウト後に表示するエラーダイアログ（実際の Win32 ウィンドウのクライアント領域に内容だけ表示）
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
            + '<div class="result-stat-row"><span class="result-stat-label">生存時間</span><span class="result-stat-value">' + formatTime(data.elapsedTime) + '</span></div>'
            + '<div class="result-stat-row"><span class="result-stat-label">被ダメージ</span><span class="result-stat-value">' + Math.floor(data.totalDamageTaken || 0) + '</span></div>'
            + '</div>'
            + '<div class="result-divider"></div>'
            + '<div class="result-btn-row">'
            + '<button class="result-btn" data-action="retry">再試行</button>'
            + '<button class="result-btn" data-action="title">タイトルへ</button>'
            + '</div>'
            + '</div>';
    }

    // ---- BSOD プログレスバーのアニメーション（JS 駆動 / 4秒で 0→100%）----

    function startBsodAnimation() {
        var fill = document.getElementById('bsod-fill');
        var pct  = document.getElementById('bsod-pct');
        if (!fill || !pct) return;

        var progress = 0;
        var timer = setInterval(function () {
            progress += 1;
            fill.style.width = progress + '%';
            pct.textContent  = String(progress);
            if (progress >= 100) {
                clearInterval(timer);
                // 0.4 秒停止後に BSOD をフェードアウトしてダイアログを表示
                setTimeout(revealLoseDialog, 400);
            }
        }, 40); // 40ms × 100 = 4000ms
    }

    function revealLoseDialog() {
        var overlay = document.getElementById('bsod-overlay');
        var dialog  = document.getElementById('lose-dialog');
        if (!overlay) return;

        overlay.classList.add('fade-out');
        // フェードアウト完了後にダイアログを表示しボタンを有効化
        setTimeout(function () {
            overlay.style.display = 'none';
            if (dialog) {
                dialog.style.display       = 'flex';
                dialog.style.flexDirection = 'column';
            }
            setupButtons();
        }, 600);
    }

    // ---- 描画 ----

    function render(data) {
        var root = document.getElementById('result-root');
        if (data.isVictory) {
            root.innerHTML = renderWin(data);
            setupButtons();
        } else {
            root.innerHTML = renderLose(data);
            // 敗北はボタン有効化を revealLoseDialog 内で行う
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

    // ---- メッセージハンドラ ----

    window.onMessageFromGame = function (data) {
        if (data.type === 'resultData') {
            render(data);
        }
    };

    // C++ にリザルトデータをリクエスト
    sendToGame({ type: 'requestResult' });
}());
