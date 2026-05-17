var descriptions = {
    EASY:   'ゲーム初心者向け。敵の攻撃力・防御力が低く、余裕を持って攻略できます。',
    NORMAL: '標準的な難易度。バランスの取れた戦闘が楽しめます。',
    HARD:   '⚠ 上級者向け。敵が強力になり、戦略的なプレイが求められます。'
};

var currentDiff = 'NORMAL';

function selectDiff(d) {
    if (d === 'HARD') {
        sendToGame({ type: 'confirmHard' });
        return;
    }
    applyDiff(d);
}

function applyDiff(d) {
    currentDiff = d;
    document.getElementById('btn-easy').className   = 'diff-btn';
    document.getElementById('btn-normal').className = 'diff-btn';
    document.getElementById('btn-hard').className   = 'diff-btn danger';

    if (d === 'EASY')        document.getElementById('btn-easy').classList.add('active-easy');
    else if (d === 'NORMAL') document.getElementById('btn-normal').classList.add('active-normal');
    else if (d === 'HARD')   document.getElementById('btn-hard').classList.add('active-hard');

    var desc = document.getElementById('diff-desc');
    desc.textContent = descriptions[d];
    desc.className = d === 'HARD' ? 'diff-desc warn' : 'diff-desc';
    sendToGame({ type: 'difficultyChanged', difficulty: d });
}

function onMessageFromGame(data) {
    if (data.type === 'hardConfirmed') {
        applyDiff('HARD');
    }
}
