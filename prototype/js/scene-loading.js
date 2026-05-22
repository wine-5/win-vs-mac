/* ===================================================
   LOADING — コマンドプロンプト Style
   =================================================== */

function getCmdLines() {
    const diff = (typeof selectedDifficulty !== 'undefined' ? selectedDifficulty : 'NORMAL').toLowerCase();
    const job  = typeof selectedJob !== 'undefined' ? selectedJob : '剣士';
    return [
        // [0] typewriter line — the command itself
        { text: `C:\\DUNGEON\\STAGE01> dungeon_load.exe --stage 1 --difficulty ${diff}`, cls: 'cmd-cmd', tw: true },
        // [1..] lines that appear sequentially (delay = time after previous)
        { text: `DungeonLoader v2.4.1  Copyright (C) SYSTEM Corp.`, cls: 'cmd-dim', delay: 65 },
        { text: '', delay: 55 },
        { text: '> システムを初期化しています...',                   delay: 125 },
        { text: '  [  OK  ] SystemCore initialized',               cls: 'cmd-ok', delay: 195 },
        { text: '> マップデータを読み込んでいます...',              delay: 125 },
        { text: '  [  OK  ] map_stage01.dat loaded  (2,048 KB)',   cls: 'cmd-ok', delay: 185 },
        { text: '> エネミーデータを解析しています...',             delay: 125 },
        { text: '  [  OK  ] enemy_table.bin loaded  (12 entries)', cls: 'cmd-ok', delay: 185 },
        { text: '> プレイヤーデータを適用しています...',           delay: 125 },
        { text: `  [  OK  ] player.dat applied  (job: ${job})`,    cls: 'cmd-ok', delay: 185 },
        { text: `> 難易度を設定しています...`,                     delay: 125 },
        { text: `  [  OK  ] difficulty = ${diff.toUpperCase()}`,   cls: 'cmd-ok', delay: 165 },
        { text: `WARNING: dungeon_rng seed = 0xDEADBEEF`,          cls: 'cmd-warn', delay: 210 },
        { text: '', delay: 85 },
        { text: '> ダンジョンに入室します...',                     delay: 145 },
    ];
}

function startBoot() {
    if (loadingTimer) { clearInterval(loadingTimer); loadingTimer = null; }

    const output = document.getElementById('cmd-output');
    if (!output) return;
    output.innerHTML = '';

    const lines   = getCmdLines();
    const cmdLine = lines[0];
    const rest    = lines.slice(1);

    // Step 1: typewriter for the command line
    typeCmdLine(output, cmdLine.text, cmdLine.cls, 6, () => {
        // Step 2: append remaining lines with cumulative delays
        let acc = 80; // brief pause after typewriter
        rest.forEach(line => {
            acc += (line.delay || 0);
            const d = acc;
            setTimeout(() => appendCmdLine(output, line.text, line.cls || ''), d);
        });

        // Step 3: progress bar (starts after all lines)
        acc += 130;
        setTimeout(() => startCmdProgress(output, () => {
            applySelectionsToIngame();
            showScene('ingame');
        }), acc);
    });
}

/* ── Typewriter effect for a single line ── */
function typeCmdLine(container, text, cls, msPerChar, onComplete) {
    const div = document.createElement('div');
    div.className = 'cmd-line' + (cls ? ' ' + cls : '');
    div.textContent = '';
    container.appendChild(div);

    let i = 0;
    (function next() {
        if (i < text.length) {
            div.textContent = text.slice(0, ++i);
            setTimeout(next, msPerChar);
        } else {
            if (onComplete) onComplete();
        }
    })();
}

/* ── Append a plain line instantly ── */
function appendCmdLine(container, text, cls) {
    const div = document.createElement('div');
    div.className = 'cmd-line' + (cls ? ' ' + cls : '');
    div.textContent = text;
    container.appendChild(div);
}

/* ── Animated progress bar ── */
function startCmdProgress(container, onComplete) {
    appendCmdLine(container, '', '');

    const row   = document.createElement('div');
    row.className = 'cmd-progress-row cmd-line';

    const track = document.createElement('div');
    track.className = 'cmd-progress-track';
    const fill = document.createElement('div');
    fill.className = 'cmd-progress-fill';
    track.appendChild(fill);

    const pct = document.createElement('span');
    pct.className = 'cmd-progress-pct';
    pct.textContent = '0%';

    row.appendChild(track);
    row.appendChild(pct);
    container.appendChild(row);

    let progress = 0;
    loadingTimer = setInterval(() => {
        progress += 4 + Math.random() * 5;
        if (progress >= 100) {
            progress = 100;
            clearInterval(loadingTimer);
            loadingTimer = null;
            fill.style.width = '100%';
            pct.textContent = '100%';
            setTimeout(onComplete, 360);
            return;
        }
        fill.style.width = progress + '%';
        pct.textContent = Math.floor(progress) + '%';
    }, 32);
}

/* ── Apply selections to in-game HUD ── */
function applySelectionsToIngame() {
    const jobEl   = document.getElementById('hud-job');
    const skillEl = document.getElementById('hud-skill');
    const typeEl  = document.getElementById('stat-type');
    const filesEl = document.getElementById('equipped-files');

    if (jobEl)   jobEl.textContent   = `${selectedJob === '剣士' ? '⚔' : selectedJob === '魔法使い' ? '🔮' : '🗡'} ${selectedJob}`;
    if (skillEl) skillEl.textContent = `必殺: ${selectedJobSkill}`;
    if (typeEl)  typeEl.textContent  = selectedJobType.toUpperCase();

    if (filesEl && selectedFiles.length > 0) {
        const icons = {
            '.exe': '⚙', '.dll': '⚙', '.bat': '⚙', '.txt': '📄', '.pdf': '📄',
            '.docx': '📝', '.png': '🖼', '.jpg': '🖼', '.bmp': '🖼',
            '.mp3': '🎵', '.wav': '🎵', '.zip': '📦', '.7z': '📦', '.rar': '📦'
        };
        filesEl.innerHTML = selectedFiles.map(f =>
            `<div>${icons[f.ext] || '❓'} ${f.name} <span style="color:var(--neon-cyan)">${f.bonus}</span></div>`
        ).join('');
    }
}

