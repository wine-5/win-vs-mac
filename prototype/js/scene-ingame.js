/* ===== INGAME ===== */
function doAttack() {
    if (!gameState.isRunning || gameState.isPaused) return;

    // プレイヤーの攻撃範囲内の敵を判定
    const closestEnemy = gameState.enemies.reduce((closest, enemy) => {
        const dist = getDistance(gameState.player.position, enemy.position);
        if (dist <= gameState.player.attackRange) {
            if (!closest || dist < getDistance(gameState.player.position, closest.position)) {
                return enemy;
            }
        }
        return closest;
    }, null);

    if (closestEnemy) {
        attackEnemy(closestEnemy.id);
    }

    // 攻撃エフェクト表示
    const iso = document.getElementById('dungeon-iso');
    const effect = document.createElement('div');
    effect.className = 'attack-effect';
    effect.style.left = (gameState.player.position.x + Math.random() * 100 - 50) + 'px';
    effect.style.top = (gameState.player.position.y + Math.random() * 100 - 50) + 'px';
    iso.appendChild(effect);
    setTimeout(() => effect.remove(), 400);
}

function doSpecial() {
    // 未実装：必殺技はプロトタイプでは不要
}

function takeDamage() {
    const overlay = document.getElementById('damage-overlay');
    overlay.classList.remove('flashing');
    void overlay.offsetWidth;
    overlay.classList.add('flashing');
    setTimeout(() => overlay.classList.remove('flashing'), 400);
}

// パウズ機能の改良版
function togglePauseUI() {
    if (gameState.isRunning) {
        togglePause();
    }
}

// ATTACKボタンのクリックハンドラ
document.addEventListener('click', (e) => {
    if (e.target.textContent === 'ATTACK' && e.target.classList.contains('action-btn-attack')) {
        doAttack();
    }
});
