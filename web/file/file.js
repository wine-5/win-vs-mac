'use strict';

const EXT_ICON  = {
    Executable: '⚙️',
    Document:   '📝',
    Image:      '🖻️',
    Audio:      '🎵',
    Archive:    '📦',
    Unknown:    '❓',
};
const EXT_LABEL = { Executable: 'EXE', Document: 'DOC', Image: 'IMG', Audio: 'AUD', Archive: 'ARC', Unknown: '???' };
const EXT_CLASS = { Executable: 'exe', Document: 'doc', Image: 'img', Audio: 'aud', Archive: 'arc', Unknown: 'unk' };
const EXT_DESC  = {
    Executable: 'ATK +10',
    Document:   'SPD +1.5',
    Image:      'DEF +8',
    Audio:      'HP +20',
    Archive:    'ATK+3 HP+5 DEF+3',
    Unknown:    '(射程 +20)',
};

const slots = [
    { isEmpty: true, fileName: null, extType: null },
    { isEmpty: true, fileName: null, extType: null },
    { isEmpty: true, fileName: null, extType: null },
];

// 前回のスロット状態（空→埋まり の検出用）
const prevEmpty = [true, true, true];

let selectedSlot = null;

function selectSlot(i) {
    selectedSlot = i;
    document.querySelectorAll('.file-row').forEach(function (row) {
        const isSelected = parseInt(row.dataset.slot, 10) === selectedSlot;
        row.classList.toggle('selected', isSelected);
        if (isSelected) {
            row.classList.remove('pulse');
            void row.offsetWidth;
            row.classList.add('pulse');
            row.addEventListener('animationend', function () {
                row.classList.remove('pulse');
            }, { once: true });
        }
    });
    updateStatus();
    sendToGame({ type: 'slotSelected', slot: i });
}

function updateBonusHighlights() {
    const activeExts = new Set();
    slots.forEach(function (s) {
        if (!s.isEmpty && s.extType) activeExts.add(s.extType);
    });
    document.querySelectorAll('.bonus-entry[data-ext]').forEach(function (el) {
        const wasActive = el.classList.contains('active');
        const isActive  = activeExts.has(el.dataset.ext);
        if (isActive && !wasActive) {
            el.classList.add('active');
            el.style.animation = 'none';
            void el.offsetWidth;
            el.style.animation = '';
        } else if (!isActive) {
            el.classList.remove('active');
        }
    });
}

function updateStatus() {
    const el = document.getElementById('status-text');
    if (!el) return;
    if (selectedSlot === null) {
        el.textContent = 'ファイルをクリックして選択';
    } else if (slots[selectedSlot].isEmpty) {
        el.textContent = 'Slot ' + (selectedSlot + 1) + ' ─ クリックしてファイルを選択';
    } else {
        el.textContent = 'Slot ' + (selectedSlot + 1) + ' を選択中';
    }
}

function renderSlots() {
    const list = document.getElementById('file-list');
    list.innerHTML = '';
    slots.forEach(function (s, i) {
        const et         = s.extType || 'Unknown';
        const isEmpty    = s.isEmpty;
        const justLoaded = prevEmpty[i] && !isEmpty;
        const row        = document.createElement('div');
        row.className    = 'file-row' + (i === selectedSlot ? ' selected' : '') + (justLoaded ? ' anim-in' : '');
        row.dataset.slot = i;
        row.onclick      = function () { selectSlot(i); };

        const badge = isEmpty ? '<span></span>' :
            '<span class="ext-badge ' + (EXT_CLASS[et] || 'unk') + '">' + (EXT_LABEL[et] || '???') + '</span>';

        const bonusText = (!isEmpty && EXT_DESC[et]) ? EXT_DESC[et] : null;
        const bonus = bonusText
            ? '<span class="bonus-text has-bonus">' + bonusText + '</span>'
            : '<span class="bonus-text">—</span>';

        row.innerHTML =
            '<div class="file-name-cell">' +
                '<span class="file-icon">' + (isEmpty ? '📄' : (EXT_ICON[et] || '📄')) + '</span>' +
                '<span class="file-name' + (isEmpty ? ' empty' : '') + '">' +
                    (isEmpty ? '─ 未選択 ─' : (s.fileName || '—')) +
                '</span>' +
            '</div>' +
            badge + bonus;
        list.appendChild(row);

        prevEmpty[i] = isEmpty;
    });
    updateStatus();
    updateBonusHighlights();
}

function onMessageFromGame(data) {
    if (data.type === 'refresh' && Array.isArray(data.slots)) {
        data.slots.forEach(function (info) {
            if (info.slot >= 0 && info.slot < 3) slots[info.slot] = info;
        });
        renderSlots();
    }
}

renderSlots();
