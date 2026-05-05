/* ===== TASKBAR CLOCK ===== */
function updateClock() {
    const now = new Date();
    const h = String(now.getHours()).padStart(2, '0');
    const m = String(now.getMinutes()).padStart(2, '0');
    const el = document.getElementById('taskbar-time');
    if (el) el.textContent = `${h}:${m}`;
}

updateClock();
setInterval(updateClock, 10000);
