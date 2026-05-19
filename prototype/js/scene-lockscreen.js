/* ===== LOCK SCREEN ===== */

let lockClockTimer = null;

function startLockScreen() {
    // Reset slide-up state on every entry
    const content = document.getElementById('lockscreen-content');
    const bottom  = document.getElementById('lockscreen-bottom');
    if (content) content.classList.remove('slide-up');
    if (bottom)  bottom.classList.remove('slide-up');

    // Start / restart clock
    updateLockClock();
    if (lockClockTimer) clearInterval(lockClockTimer);
    lockClockTimer = setInterval(updateLockClock, 1000);

    // Create particles only if the container is empty
    const pc = document.getElementById('particles-container');
    if (pc && pc.children.length === 0) {
        createParticles();
    }
}

function updateLockClock() {
    const now     = new Date();
    const hours   = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const days    = ['日曜日', '月曜日', '火曜日', '水曜日', '木曜日', '金曜日', '土曜日'];
    const dateStr = `${now.getFullYear()}年${now.getMonth() + 1}月${now.getDate()}日  ${days[now.getDay()]}`;

    const timeEl = document.getElementById('lockscreen-time');
    const dateEl = document.getElementById('lockscreen-date');
    if (timeEl) timeEl.textContent = `${hours}:${minutes}`;
    if (dateEl) dateEl.textContent = dateStr;
}

function unlockScreen() {
    const content = document.getElementById('lockscreen-content');
    const bottom  = document.getElementById('lockscreen-bottom');
    if (content) content.classList.add('slide-up');
    if (bottom)  bottom.classList.add('slide-up');

    if (lockClockTimer) {
        clearInterval(lockClockTimer);
        lockClockTimer = null;
    }

    setTimeout(() => {
        showScene('select');
    }, 580);
}
