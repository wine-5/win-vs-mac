/* ===== TITLE SCENE ===== */
document.getElementById('scene-title').addEventListener('click', function () {
    const ls = document.getElementById('lockscreen');
    if (!ls.classList.contains('sliding-up')) {
        ls.classList.add('sliding-up');
    }
});

function updateLockTime() {
    const now = new Date();
    const h = String(now.getHours()).padStart(2, '0');
    const m = String(now.getMinutes()).padStart(2, '0');
    document.getElementById('lock-time').textContent = `${h}:${m}`;
    const days = ['日', '月', '火', '水', '木', '金', '土'];
    const day = days[now.getDay()];
    document.getElementById('lock-date').textContent =
        `${now.getFullYear()}年${now.getMonth() + 1}月${now.getDate()}日 ${day}曜日`;
}

updateLockTime();
setInterval(updateLockTime, 30000);
