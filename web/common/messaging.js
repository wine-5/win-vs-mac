/**
 * JS → C++ へ JSON オブジェクトを送信する
 * @param {object} obj 送信するオブジェクト
 */
function sendToGame(obj) {
    try {
        window.chrome.webview.postMessage(JSON.stringify(obj));
    } catch (err) {
        console.error('sendToGame error:', err);
    }
}

// C++ → JS メッセージ受信。各ページで onMessageFromGame を定義して使う
window.chrome.webview.addEventListener('message', function (e) {
    try {
        const data = JSON.parse(e.data);
        if (typeof onMessageFromGame === 'function') {
            onMessageFromGame(data);
        }
    } catch (err) {
        console.error('Message listener error:', err);
    }
});

/**
 * 難易度に応じて body へ hard クラスを付け外しする
 *
 * HARD では配色を警告色へ変える。各ページの onMessageFromGame から呼ぶ
 * @param {object} data C++から届いたメッセージ
 */
function applyDifficultyTheme(data) {
    if (!data || data.type !== 'difficultyChanged') return;
    document.body.classList.toggle('hard', data.difficulty === 'HARD');
}
