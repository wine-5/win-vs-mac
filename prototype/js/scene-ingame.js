/* ===== INGAME ===== */
function doAttack() {
    const iso = document.getElementById('dungeon-iso');
    const effect = document.createElement('div');
    effect.className = 'attack-effect';
    effect.style.left = (220 + Math.random() * 160) + 'px';
    effect.style.top = (180 + Math.random() * 140) + 'px';
    iso.appendChild(effect);
    setTimeout(() => effect.remove(), 400);
}

function doSpecial() {
    for (let i = 0; i < 6; i++) setTimeout(() => doAttack(), i * 80);
}

function takeDamage() {
    const overlay = document.getElementById('damage-overlay');
    overlay.classList.remove('flashing');
    void overlay.offsetWidth;
    overlay.classList.add('flashing');
    setTimeout(() => overlay.classList.remove('flashing'), 400);
}

function togglePause() {
    isPaused = !isPaused;
    document.getElementById('pause-overlay').classList.toggle('active', isPaused);
}

document.addEventListener('keydown', e => {
    if (e.key === 'Escape' && document.getElementById('scene-ingame').classList.contains('active')) {
        togglePause();
    }
});
