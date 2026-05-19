/* ===== BIOS SCENE ===== */

const BIOS_LINES = [
    { text: '  AMI UEFI BIOS  \u2500  WIN vs MAC Dungeon System v1.0.0  ', type: 'head', delay: 0 },
    { text: '  Copyright (C) 2026  WIN vs MAC Development Team      ', type: 'head', delay: 80 },
    { text: '', delay: 140 },
    { text: 'CPU  : WIN-CORE i9-X9900K @ 5.80GHz ...................................... [OK]', type: 'ok', delay: 520 },
    { text: 'Mem  : 32768 MB DDR5-6400 ................................................. [OK]', type: 'ok', delay: 240 },
    { text: '', delay: 80 },
    { text: 'Detecting storage devices...', delay: 320 },
    { text: '  C:\\ NTFS  512 GB  (Windows OS + Dungeon Core) ....................... [OK]', type: 'ok', delay: 210 },
    { text: '  D:\\ NTFS    2 TB  (User Data + Enemy Database) ...................... [OK]', type: 'ok', delay: 210 },
    { text: '  Z:\\ ???   ??? GB  (UNKNOWN \u2500 Suspicious Files) ................. [WARN]', type: 'warn', delay: 480 },
    { text: '', delay: 80 },
    { text: 'PCI-E Devices:', delay: 180 },
    { text: '  GPU : NVIDIA GeForce RTX 4090 ........................................... [OK]', type: 'ok', delay: 170 },
    { text: '  NET : Intel AX210 WiFi 6E ............................................... [OK]', type: 'ok', delay: 170 },
    { text: '', delay: 80 },
    { text: 'DUNGEON SUBSYSTEM INITIALIZING...', type: 'bold', delay: 360 },
    { text: '  Kernel Modules      [\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588] 100% .............................. [OK]', type: 'ok', delay: 310 },
    { text: '  Enemy Database      [\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588] 100% .............................. [OK]', type: 'ok', delay: 270 },
    { text: '  Physics Engine      [\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588] 100% .............................. [OK]', type: 'ok', delay: 290 },
    { text: '  Dungeon Generator   [\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588] 100% .............................. [OK]', type: 'ok', delay: 370 },
    { text: '', delay: 100 },
    { text: '\u26a0  WARNING: Unauthorized process detected  (mac_invasion.exe)', type: 'warn', delay: 680 },
    { text: '   Countermeasures loading...', type: 'warn', delay: 400 },
    { text: '', delay: 200 },
    { text: 'System Ready.  Starting WIN vs MAC...', type: 'bold', delay: 560 },
    { text: '', delay: 80 },
    { text: '\u2500'.repeat(82), type: 'dim', delay: 80 },
    { text: ' [F2] Enter Setup   \u2502   [F12] Boot Menu   \u2502   [ESC] Skip ', type: 'dim', delay: 50 },
];

let biosTimers = [];

function startBios() {
    biosTimers.forEach(t => clearTimeout(t));
    biosTimers = [];

    const output = document.getElementById('bios-output');
    if (!output) return;
    output.innerHTML = '';

    let totalDelay = 0;
    BIOS_LINES.forEach(line => {
        totalDelay += line.delay || 0;
        const t = setTimeout(() => {
            if (!document.getElementById('scene-bios').classList.contains('active')) return;
            const div = document.createElement('div');
            div.className = 'bios-line' + (line.type ? ' ' + line.type : '');
            div.textContent = line.text;
            output.appendChild(div);
        }, totalDelay);
        biosTimers.push(t);
    });

    // Auto-advance to lock screen 900ms after last line
    totalDelay += 900;
    const finalTimer = setTimeout(() => {
        if (document.getElementById('scene-bios').classList.contains('active')) {
            showScene('lockscreen');
        }
    }, totalDelay);
    biosTimers.push(finalTimer);
}

// ESC key skips directly to lock screen
document.addEventListener('keydown', e => {
    if (e.key === 'Escape' && document.getElementById('scene-bios') &&
        document.getElementById('scene-bios').classList.contains('active')) {
        biosTimers.forEach(t => clearTimeout(t));
        biosTimers = [];
        showScene('lockscreen');
    }
});

// Auto-start when page loads with BIOS as initial active scene
document.addEventListener('DOMContentLoaded', () => {
    const biosEl = document.getElementById('scene-bios');
    if (biosEl && biosEl.classList.contains('active')) {
        startBios();
    }
});
