// ===== BSOD Animation =====
function startBSOD() {
    const overlay = document.getElementById('bsod-overlay');
    const pct = document.getElementById('bsod-pct');
    const bar = document.getElementById('bsod-bar');
    if (!overlay) return;

    overlay.classList.remove('bsod-fade-out');
    overlay.style.display = 'flex';
    pct.textContent = '0';
    bar.style.width = '0%';

    let progress = 0;
    const interval = setInterval(function () {
        progress += Math.random() * 2.5 + 0.4;
        if (progress >= 100) {
            progress = 100;
            clearInterval(interval);
            setTimeout(function () {
                overlay.classList.add('bsod-fade-out');
                overlay.addEventListener('animationend', function () {
                    overlay.style.display = 'none';
                }, { once: true });
            }, 700);
        }
        pct.textContent = Math.floor(progress);
        bar.style.width = progress + '%';
    }, 60);
}

// Wrap showScene to trigger BSOD on result-lose
(function () {
    const orig = window.showScene;
    window.showScene = function (name) {
        orig(name);
        if (name === 'result-lose') {
            setTimeout(startBSOD, 80);
        }
    };
}());
