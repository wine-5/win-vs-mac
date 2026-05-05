/* ===== JOB SELECT ===== */
function selectJob(el, job, type, skill) {
    document.querySelectorAll('.job-item').forEach(j => j.classList.remove('selected'));
    el.classList.add('selected');
    selectedJob = job;
    selectedJobType = type;
    selectedJobSkill = skill;
}

function confirmJob() {
    alert(`職業「${selectedJob}」を選択しました\n攻撃タイプ: ${selectedJobType}\n必殺技: ${selectedJobSkill}`);
    closeWindow('win-job');
}

/* ===== FILE SELECT ===== */
function toggleExFile(el, name, ext, bonus) {
    if (el.classList.contains('selected')) {
        el.classList.remove('selected');
        selectedFiles = selectedFiles.filter(f => f.name !== name);
    } else {
        if (selectedFiles.length >= 3) {
            document.getElementById('ex-status').textContent = '⚠ 最大3つまで選択できます';
            document.getElementById('ex-status').style.color = 'red';
            return;
        }
        el.classList.add('selected');
        selectedFiles.push({ name, ext, bonus });
    }
    const status = document.getElementById('ex-status');
    status.style.color = '';
    status.textContent = selectedFiles.length === 0
        ? 'ファイルを選択してください（最大3つ）'
        : `${selectedFiles.length}個選択中: ${selectedFiles.map(f => f.name).join(', ')}`;
}

function goToGame() {
    if (selectedFiles.length === 0) {
        alert('ファイルを1つ以上選択してください');
        return;
    }
    showScene('loading');
}
