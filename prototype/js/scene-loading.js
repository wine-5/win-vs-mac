/* ===== LOADING (BOOT) ===== */
const bootMessages = [
    'フォルダ構造を読み込んでいます...',
    'ファイルサイズを解析しています...',
    '難易度を計算しています...',
    'ダンジョンを生成しています...',
    'プレイヤーパラメータを適用しています...',
    '敵の配置を決定しています...',
    '起動完了...',
];

function startBoot() {
    if (loadingTimer) clearInterval(loadingTimer);
    let idx = 0;
    const statusEl = document.getElementById('boot-status');
    statusEl.textContent = bootMessages[0];
    loadingTimer = setInterval(() => {
        idx++;
        if (idx >= bootMessages.length) {
            clearInterval(loadingTimer);
            setTimeout(() => {
                applySelectionsToIngame();
                showScene('ingame');
            }, 600);
            return;
        }
        statusEl.textContent = bootMessages[idx];
    }, 900);
}

function applySelectionsToIngame() {
    document.getElementById('hud-job').textContent = `${selectedJob === '剣士' ? '⚔' : selectedJob === '魔法使い' ? '🔮' : '🗡'} ${selectedJob}`;
    document.getElementById('hud-skill').textContent = `必殺: ${selectedJobSkill}`;
    document.getElementById('stat-type').textContent = selectedJobType.toUpperCase();
    if (selectedFiles.length > 0) {
        const icons = { '.exe': '⚙', '.dll': '⚙', '.bat': '⚙', '.txt': '📄', '.pdf': '📄', '.docx': '📝', '.png': '🖼', '.jpg': '🖼', '.bmp': '🖼', '.mp3': '🎵', '.wav': '🎵', '.zip': '📦', '.7z': '📦', '.rar': '📦' };
        document.getElementById('equipped-files').innerHTML = selectedFiles.map(f =>
            `<div>${icons[f.ext] || '❓'} ${f.name} <span style="color:var(--neon-cyan)">${f.bonus}</span></div>`
        ).join('');
    }
}
