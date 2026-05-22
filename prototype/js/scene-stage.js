/* ===================================================
   STAGE SELECT
   =================================================== */

const JOB_DATA = {
    '剣士': {
        en: 'Warrior', type: 'slash', skill: '全方位斬り',
        hp: 120, atk: 45, def: 35, spd: 30, icon: '⚔',
        desc: '近接戦闘を得意とする前衛職。前方扇形への斬撃で複数の敵をまとめて攻撃できる。防御力が高く初心者にもおすすめ。'
    },
    '魔法使い': {
        en: 'Mage', type: 'magic', skill: '巨大魔法弾',
        hp: 80, atk: 60, def: 20, spd: 40, icon: '🔮',
        desc: '遠距離から魔法弾を放つ後衛職。攻撃力は最高だが防御力が低い。敵との距離を保ちながら戦うのがコツ。'
    },
    '忍者': {
        en: 'Ninja', type: 'pierce', skill: '分身一斉攻撃',
        hp: 90, atk: 50, def: 25, spd: 55, icon: '🗡',
        desc: '素早い機動力で敵を翻弄する高速アタッカー。移動速度が最高で、分身による広範囲攻撃が得意。'
    }
};

const DIFFICULTY_DATA = {
    'EASY':   { desc: '敵が弱く、HPが多め。ゲームに慣れていない方向け。',  hpMult: 1.3,  atkMult: 0.85 },
    'NORMAL': { desc: '標準的な難易度。バランスよく楽しめます。',           hpMult: 1.0,  atkMult: 1.0  },
    'HARD':   { desc: 'HPが少なく敵が強い。上級者向け。覚悟して挑め。',    hpMult: 0.75, atkMult: 1.25 }
};

const STAT_MAX = { hp: 200, atk: 100, def: 80, spd: 100 };

/* ===== JOB SELECT ===== */
function selectJob(el, jobName) {
    document.querySelectorAll('.job-item').forEach(j => j.classList.remove('selected'));
    el.classList.add('selected');

    const data = JOB_DATA[jobName];
    selectedJob     = jobName;
    selectedJobType = data.type;
    selectedJobSkill = data.skill;

    document.getElementById('job-desc-name').textContent  = `${jobName} / ${data.en}`;
    document.getElementById('job-desc-text').textContent  = data.desc;
    document.getElementById('job-desc-skill').textContent = data.skill;
    document.getElementById('job-desc-type').textContent  = data.type;

    updateParams();
}

/* ===== DIFFICULTY SELECT ===== */
function setDifficulty(diff) {
    selectedDifficulty = diff;
    document.querySelectorAll('.diff-btn').forEach(b => b.classList.remove('active'));
    document.getElementById(`diff-${diff.toLowerCase()}`).classList.add('active');
    document.getElementById('difficulty-desc').textContent = DIFFICULTY_DATA[diff].desc;
}

/* ===== FILE SELECT (Slot-based) ===== */
let selectedSlotIndex = -1;

function selectFileSlot(slotIndex) {
    // C++ 側のファイル選択ダイアログを呼び出す（実装時）
    // ここではプロトタイプなので、デモ用のダイアログをシミュレート
    selectedSlotIndex = slotIndex;

    // デモ用：ファイル選択をシミュレート（実装時は C++ の GetOpenFileName を呼び出す）
    const demoFiles = [
        { name: 'project_final.exe', stat: 'atk', amount: 10 },
        { name: 'readme.txt', stat: 'spd', amount: 5 },
        { name: 'screenshot.png', stat: 'def', amount: 8 },
        { name: 'bgm.mp3', stat: 'hp', amount: 20 },
        { name: 'backup.zip', stat: 'all', amount: 5 },
        { name: 'data.dll', stat: 'atk', amount: 10 },
        { name: 'notes.docx', stat: 'spd', amount: 5 },
        { name: 'mystery.???', stat: 'mystery', amount: 0 }
    ];

    // ダイアログが閉じたと仮定してファイルを選択
    if (selectedSlotIndex !== -1 && selectedSlotIndex < 3) {
        const selectedFile = demoFiles[selectedSlotIndex];
        updateFileSlot(selectedFile.name, selectedFile.stat, selectedFile.amount);
    }
}

function updateFileSlot(name, stat, amount) {
    if (selectedSlotIndex === -1) return;

    // Remove old file if exists
    const oldFile = selectedFiles.find(f => f.slotIndex === selectedSlotIndex);
    if (oldFile) {
        selectedFiles = selectedFiles.filter(f => f.slotIndex !== selectedSlotIndex);
    }

    // Add new file
    selectedFiles.push({ name, stat, amount, slotIndex: selectedSlotIndex });

    // Update slot display
    const slotDisplayId = `slot-selected-${selectedSlotIndex + 1}`;
    const slotDisplayEl = document.getElementById(slotDisplayId);
    if (slotDisplayEl) {
        slotDisplayEl.textContent = `${name} (${stat === 'all' ? 'ALL +' + amount : stat.toUpperCase() + ' +' + amount})`;
        slotDisplayEl.style.color = '#00a4ef';
    }

    updateParams();
    selectedSlotIndex = -1;
}

function selectFileForSlot(name, stat, amount) {
    // 古い UI から呼び出される可能性があるため互換性を保つ
    updateFileSlot(name, stat, amount);
}

/* ===== PARAMS UPDATE ===== */
function updateParams() {
    const job  = JOB_DATA[selectedJob];

    let stats = { hp: job.hp, atk: job.atk, def: job.def, spd: job.spd };

    for (const f of selectedFiles) {
        if (f.stat === 'all') {
            stats.hp += f.amount; stats.atk += f.amount;
            stats.def += f.amount; stats.spd += f.amount;
        } else if (f.stat !== 'mystery') {
            stats[f.stat] += f.amount;
        }
    }

    document.getElementById('param-job-icon').textContent = job.icon;
    document.getElementById('param-job-name').textContent = `${selectedJob} / ${job.en}`;
    document.getElementById('param-skill-name').textContent = job.skill;

    for (const key of ['hp', 'atk', 'def', 'spd']) {
        const pct = Math.min(100, Math.round((stats[key] / STAT_MAX[key]) * 100));
        document.getElementById(`bar-${key}`).style.width = `${pct}%`;
        document.getElementById(`val-${key}`).textContent = stats[key];
    }

    const equipList = document.getElementById('param-equip-list');
    if (selectedFiles.length === 0) {
        equipList.innerHTML = '<div class="param-equip-empty">— なし —</div>';
    } else {
        equipList.innerHTML = selectedFiles.map(f => {
            let bonusLabel;
            if (f.stat === 'mystery') {
                bonusLabel = '<span style="color:#c42b1c">???</span>';
            } else if (f.stat === 'all') {
                bonusLabel = `ALL +${f.amount}`;
            } else {
                bonusLabel = `${f.stat.toUpperCase()} +${f.amount}`;
            }
            return `<div class="param-equip-item">
                <span class="param-equip-name">${f.name}</span>
                <span class="param-equip-bonus">${bonusLabel}</span>
            </div>`;
        }).join('');
    }
}

function goToGame() {
    showScene('loading');
}
