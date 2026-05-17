/**
 * JS → C++ へ JSON オブジェクトを送信する
 * @param {object} obj 送信するオブジェクト
 */
function sendToGame(obj) {
    window.chrome.webview.postMessage(JSON.stringify(obj));
}

// C++ → JS メッセージ受信。各ページで onMessageFromGame を定義して使う
window.chrome.webview.addEventListener('message', function (e) {
    try {
        const data = JSON.parse(e.data);
        if (typeof onMessageFromGame === 'function') {
            onMessageFromGame(data);
        }
    } catch (_) {}
});
