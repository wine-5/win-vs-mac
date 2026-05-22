// ===== Game State Management =====
const gameState = {
    isRunning: false,
    isPaused: false,
    gameTime: 0,

    // Player data
    player: {
        hp: 10000,
        maxHp: 100,
        atk: 45, // 職業・ファイル補正後の値
        def: 10,
        spd: 12, // 移動速度を5から12に上昇
        attackType: 'slash', // slash, magic, pierce
        attackRange: 0.15, // 割合（0-1）
        position: { x: 0.5, y: 0.5 }, // 割合（0-1）: 中央
        lastAttackTime: 0,
        attackCooldown: 0.5
    },

    // Enemy data
    enemies: [],
    enemySpawnTimes: [0, 3, 8], // 敵の出現時刻（秒）
    enemySpawned: [false, false, false],

    // Game flow
    difficulty: 'NORMAL',
    roomNumber: 1,
    floorNumber: 1
};

// ===== Enemy Types =====
const enemyTypes = {
    safari: {
        name: 'Safari',
        icon: '🦁',
        weakness: 'pierce',
        maxHp: 30,
        atk: 15,
        def: 3,
        spd: 6,
        attackRange: 0.12, // 割合（0-1）
        detectionRange: 0.35,
        attackCooldown: 1.5,
        behavior: 'ranged' // 遠距離型
    },
    xcode: {
        name: 'Xcode',
        icon: '⚙️',
        weakness: 'slash',
        maxHp: 40,
        atk: 20,
        def: 5,
        spd: 2,
        attackRange: 0.08,
        detectionRange: 0.25,
        attackCooldown: 2.0,
        behavior: 'melee' // 近接型
    },
    macbook: {
        name: 'MacBook.exe',
        icon: '🖥️',
        weakness: 'magic',
        maxHp: 100,
        atk: 30,
        def: 8,
        spd: 3,
        attackRange: 0.1,
        detectionRange: 0.4,
        attackCooldown: 1.8,
        behavior: 'boss' // ボス型
    }
};

// ===== Damage Calculation =====
function calculateDamage(attacker, defender) {
    const baseDamage = Math.max(0, attacker.atk - defender.def);
    const multiplier = getTypeMultiplier(attacker.attackType, defender.weakness);
    const damage = Math.round(baseDamage * multiplier);
    return Math.max(1, damage);
}

function getTypeMultiplier(attackType, defenseWeakness) {
    const typeChart = {
        slash: { slash: 1.0, pierce: 0.5, magic: 1.5 },
        pierce: { slash: 1.5, pierce: 1.0, magic: 0.5 },
        magic: { slash: 0.5, pierce: 1.5, magic: 1.0 }
    };
    return typeChart[attackType]?.[defenseWeakness] || 1.0;
}

// ===== Enemy Management =====
function spawnEnemy(type, position) {
    const enemyType = enemyTypes[type];
    if (!enemyType) return null;

    const enemy = {
        id: 'enemy-' + Date.now() + Math.random(),
        type: type,
        ...enemyType,
        hp: enemyType.maxHp,
        position: position,
        lastAttackTime: 0,
        targetPlayer: true,
        stateDuration: 0,
        currentState: 'moving' // moving, attacking, dying
    };

    gameState.enemies.push(enemy);
    addEnemyUI(enemy);
    return enemy;
}

function updateEnemies(deltaTime) {
    // 敵の出現タイミング判定
    for (let i = 0; i < gameState.enemySpawnTimes.length; i++) {
        if (!gameState.enemySpawned[i] && gameState.gameTime >= gameState.enemySpawnTimes[i]) {
            gameState.enemySpawned[i] = true;

            if (i === 0) {
                spawnEnemy('safari', { x: 0.2, y: 0.3 }); // 左奥
            } else if (i === 1) {
                spawnEnemy('xcode', { x: 0.8, y: 0.4 }); // 右奥
            } else if (i === 2) {
                spawnEnemy('macbook', { x: 0.5, y: 0.15 }); // 中央奥（ボス）
            }
        }
    }

    // 敵のAI更新
    gameState.enemies = gameState.enemies.filter(enemy => enemy.hp > 0);

    for (const enemy of gameState.enemies) {
        updateEnemyAI(enemy, deltaTime);
    }
}

function updateEnemyAI(enemy, deltaTime) {
    const distToPlayer = getDistance(enemy.position, gameState.player.position);

    // 敵の行動パターン
    if (distToPlayer <= enemy.detectionRange) {
        if (distToPlayer <= enemy.attackRange) {
            // 攻撃範囲内
            attemptEnemyAttack(enemy);
        } else {
            // 移動
            moveEnemyTowardPlayer(enemy, deltaTime);
        }
    } else {
        // 検知範囲外で待機
        enemy.currentState = 'idle';
    }

    updateEnemyUI(enemy);
}

function moveEnemyTowardPlayer(enemy, deltaTime) {
    const dx = gameState.player.position.x - enemy.position.x;
    const dy = gameState.player.position.y - enemy.position.y;
    const dist = Math.sqrt(dx * dx + dy * dy);

    if (dist > 0) {
        const moveDistance = enemy.spd * 0.0005 * deltaTime; // 割合での移動速度
        const ratio = moveDistance / dist;
        enemy.position.x += dx * ratio;
        enemy.position.y += dy * ratio;
        enemy.currentState = 'moving';
    }
}

function attemptEnemyAttack(enemy) {
    const now = gameState.gameTime;
    if (now - enemy.lastAttackTime >= enemy.attackCooldown) {
        damagePlayer(enemy);
        enemy.lastAttackTime = now;
        enemy.currentState = 'attacking';
    }
}

// ===== Player Actions =====
function damagePlayer(attacker) {
    const damage = calculateDamage(attacker, gameState.player);
    gameState.player.hp = Math.max(0, gameState.player.hp - damage);

    showDamageText(gameState.player.position, damage, false);
    showDamageFlash();
    updatePlayerUI();

    if (gameState.player.hp <= 0) {
        endGame('defeat');
    }
}

function showDamageFlash() {
    const overlay = document.getElementById('damage-overlay');
    if (!overlay) return;
    overlay.classList.remove('flashing');
    void overlay.offsetWidth;
    overlay.classList.add('flashing');
}

function attackEnemy(enemyId) {
    const enemy = gameState.enemies.find(e => e.id === enemyId);
    if (!enemy) return;

    const damage = calculateDamage(
        { ...gameState.player, attackType: gameState.player.attackType },
        { weakness: enemy.weakness, def: enemy.def }
    );

    enemy.hp = Math.max(0, enemy.hp - damage);
    showDamageText(enemy.position, damage, true);
    updateEnemyUI(enemy);

    // ボス倒判定
    if (enemy.type === 'macbook' && enemy.hp <= 0) {
        endGame('victory');
    }
}

// ===== Player Input =====
const inputState = {
    up: false,
    down: false,
    left: false,
    right: false
};

document.addEventListener('keydown', (e) => {
    if (!gameState.isRunning || gameState.isPaused) return;

    if (e.key.toLowerCase() === 'w') inputState.up = true;
    if (e.key.toLowerCase() === 's') inputState.down = true;
    if (e.key.toLowerCase() === 'a') inputState.left = true;
    if (e.key.toLowerCase() === 'd') inputState.right = true;
    if (e.key === 'Escape') togglePause();
});

document.addEventListener('keyup', (e) => {
    if (e.key.toLowerCase() === 'w') inputState.up = false;
    if (e.key.toLowerCase() === 's') inputState.down = false;
    if (e.key.toLowerCase() === 'a') inputState.left = false;
    if (e.key.toLowerCase() === 'd') inputState.right = false;
});

function updatePlayerMovement(deltaTime) {
    const moveSpeed = gameState.player.spd * 0.0005; // 割合での移動速度
    const moveDistance = moveSpeed * deltaTime;

    if (inputState.up) gameState.player.position.y -= moveDistance;
    if (inputState.down) gameState.player.position.y += moveDistance;
    if (inputState.left) gameState.player.position.x -= moveDistance;
    if (inputState.right) gameState.player.position.x += moveDistance;

    // 画面範囲内に制限（割合: 0.15-0.85）
    gameState.player.position.x = Math.max(0.15, Math.min(0.85, gameState.player.position.x));
    gameState.player.position.y = Math.max(0.2, Math.min(0.8, gameState.player.position.y));

    updatePlayerUI();
}

// ===== Game Loop =====
let lastFrameTime = Date.now();

function gameLoop() {
    if (!gameState.isRunning) {
        requestAnimationFrame(gameLoop);
        return;
    }

    const now = Date.now();
    const deltaTime = (now - lastFrameTime) / 1000; // Convert to seconds
    lastFrameTime = now;

    if (!gameState.isPaused) {
        gameState.gameTime += deltaTime;
        updatePlayerMovement(deltaTime);
        updateEnemies(deltaTime);
    }

    updatePlayerUI();

    requestAnimationFrame(gameLoop);
}

// ===== UI Updates =====
function updatePlayerUI() {
    const playerSprite = document.querySelector('.player-sprite');
    const iso = document.getElementById('dungeon-iso');

    console.log('updatePlayerUI called. playerSprite:', !!playerSprite, 'iso:', !!iso);

    if (!playerSprite) {
        console.warn('playerSprite not found');
    }
    if (!iso) {
        console.warn('dungeon-iso not found');
    }

    const hpPercent = (gameState.player.hp / gameState.player.maxHp) * 100;
    const hpBar = document.querySelector('.hud-bar.hp');
    if (hpBar) hpBar.style.width = hpPercent + '%';

    const hpValue = document.querySelector('.hud-value');
    if (hpValue) hpValue.textContent = Math.ceil(gameState.player.hp) + ' / ' + gameState.player.maxHp;

    // 左上のパラメータパネルも更新
    const statHpDisplay = document.getElementById('stat-hp-display');
    if (statHpDisplay) statHpDisplay.textContent = Math.ceil(gameState.player.hp) + '/' + gameState.player.maxHp;

    if (playerSprite && iso) {
        const width = iso.offsetWidth;
        const height = iso.offsetHeight;
        const newLeft = (gameState.player.position.x * width) + 'px';
        const newTop = (gameState.player.position.y * height) + 'px';
        console.log('Setting sprite position. left:', newLeft, 'top:', newTop, 'iso size:', width, 'x', height);
        playerSprite.style.left = newLeft;
        playerSprite.style.top = newTop;
    }
}

function updateEnemyUI(enemy) {
    const enemyEl = document.getElementById(enemy.id);
    if (enemyEl) {
        const iso = document.getElementById('dungeon-iso');
        const rect = iso.getBoundingClientRect();
        const width = rect.width;
        const height = rect.height;
        enemyEl.style.left = (enemy.position.x * width) + 'px';
        enemyEl.style.top = (enemy.position.y * height) + 'px';

        const hpBar = enemyEl.querySelector('.enemy-hpbar-fill');
        if (hpBar) {
            const hpPercent = (enemy.hp / enemy.maxHp) * 100;
            hpBar.style.width = hpPercent + '%';
        }
    }
}

function addEnemyUI(enemy) {
    const iso = document.getElementById('dungeon-iso');
    const enemyEl = document.createElement('div');
    enemyEl.id = enemy.id;
    enemyEl.className = 'enemy-sprite';
    enemyEl.style.left = enemy.position.x + 'px';
    enemyEl.style.top = enemy.position.y + 'px';
    enemyEl.innerHTML = `
        <div style="position:relative">
            <div class="enemy-hpbar">
                <div class="enemy-hpbar-fill" style="width:100%"></div>
            </div>
            <div class="enemy-body">${enemy.icon}</div>
        </div>
    `;
    iso.appendChild(enemyEl);
}

function showDamageText(position, damage, isPlayerAttacking) {
    const iso = document.getElementById('dungeon-iso');
    const rect = iso.getBoundingClientRect();
    const width = rect.width;
    const height = rect.height;

    const damageEl = document.createElement('div');
    damageEl.className = 'damage-number';
    damageEl.textContent = damage;
    damageEl.style.left = (position.x * width) + 'px';
    damageEl.style.top = (position.y * height) + 'px';
    damageEl.style.color = isPlayerAttacking ? '#0f0' : '#f00';
    iso.appendChild(damageEl);

    setTimeout(() => damageEl.remove(), 1500);
}

// ===== Utility =====
function getDistance(pos1, pos2) {
    const dx = pos1.x - pos2.x;
    const dy = pos1.y - pos2.y;
    return Math.sqrt(dx * dx + dy * dy);
}

function endGame(result) {
    gameState.isRunning = false;
    gameState.isPaused = false;

    // UI状態をリセット
    const pauseOverlay = document.getElementById('pause-overlay');
    if (pauseOverlay) pauseOverlay.classList.remove('active');

    // ゲーム結果の表示
    setTimeout(() => {
        if (result === 'victory') {
            showScene('result-win');
        } else {
            showScene('result-lose');
        }
    }, 500);
}

function togglePause() {
    gameState.isPaused = !gameState.isPaused;
    document.getElementById('pause-overlay').classList.toggle('active', gameState.isPaused);
}

// ===== Game Start =====
function startInGame() {
    gameState.isRunning = true;
    gameState.isPaused = false;
    gameState.gameTime = 0;
    gameState.player.hp = gameState.player.maxHp;
    gameState.enemies = [];
    gameState.enemySpawned = [false, false, false];

    updatePlayerUI();
    gameLoop();
}

// Auto-start when scene becomes active
const ingameSceneEl = document.getElementById('scene-ingame');
if (ingameSceneEl) {
    const observer = new MutationObserver(() => {
        const ingameScene = document.getElementById('scene-ingame');
        if (ingameScene && ingameScene.classList.contains('active') && !gameState.isRunning) {
            console.log('Starting InGame...');
            startInGame();
        }
    });
    observer.observe(ingameSceneEl, { attributes: true });
} else {
    console.error('scene-ingame not found');
}
