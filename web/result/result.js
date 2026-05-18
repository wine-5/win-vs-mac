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

    // ---- 描画関数 ----

    function renderStats(data) {
        return '<div class="result-stats">'
            + '<div class="stat-row"><span class="stat-label">Time</span>'
            + '<span class="stat-value">' + formatTime(data.elapsedTime) + '</span></div>'
            + '<div class="stat-row"><span class="stat-label">Kills</span>'
            + '<span class="stat-value">' + (data.killCount || 0) + '</span></div>'
            + '<div class="stat-row"><span class="stat-label">Damage Taken</span>'
            + '<span class="stat-value">' + Math.floor(data.totalDamageTaken || 0) + '</span></div>'
            + '</div>';
    }

    function renderFiles(files) {
        if (!files || files.length === 0) return '';
        var tags = files.map(function (f) {
            var name = f.replace(/.*[\\/]/, '');
            return '<span class="file-tag" title="' + escapeHtml(f) + '">' + escapeHtml(name) + '</span>';
        }).join('');
        return '<div class="result-files">' + tags + '</div>';
    }

    function renderWin(data) {
        return '<div class="result-win">'
            + '<div class="result-icon">✅</div>'
            + '<h1 class="result-title">Mission Complete</h1>'
            + '<p class="result-subtitle">All enemies eliminated</p>'
            + renderStats(data)
            + renderFiles(data.usedFiles)
            + '<div class="result-buttons">'
            + '<button class="btn-retry" data-action="retry">もう一度</button>'
            + '<button class="btn-title" data-action="title">タイトルへ</button>'
            + '</div>'
            + '</div>';
    }

    function renderLose(data) {
        return '<div class="result-lose">'
            + '<div class="bsod-overlay" id="bsod-overlay">'
            + '<div class="bsod-emoji">:（</div>'
            + '<h1 class="bsod-title">PCが問題に実行して停止しました。</h1>'
            + '<p class="bsod-subtitle">KERNEL_DATA_INPAGE_ERROR</p>'
            + '<div class="bsod-progress-bar">'
            + '<div class="bsod-progress-fill" id="bsod-fill"></div>'
            + '</div>'
            + '<div class="bsod-bottom">'
            + renderStats(data)
            + renderFiles(data.usedFiles)
            + '<div class="result-buttons">'
            + '<button class="btn-retry" data-action="retry">再試行</button>'
            + '<button class="btn-title" data-action="title">タイトルへ</button>'
            + '</div>'
            + '</div>'
            + '</div>'
            + '</div>';
    }

    function render(data) {
        var root = document.getElementById('result-root');
        root.innerHTML = data.isVictory ? renderWin(data) : renderLose(data);
        setupButtons();
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
