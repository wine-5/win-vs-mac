'use strict';

const FileView = (function () {
    let fileListEl = null;
    let statusEl = null;
    let activeTypewriteTimer = null;

    // ── タイプライター演出 ────────────────────────────────────────
    function typewritePath(pathRow, text) {
        if (activeTypewriteTimer !== null) {
            clearInterval(activeTypewriteTimer);
            activeTypewriteTimer = null;
        }
        pathRow.textContent = '';
        pathRow.classList.add('visible');
        let idx = 0;
        activeTypewriteTimer = setInterval(function () {
            if (idx >= text.length) {
                clearInterval(activeTypewriteTimer);
                activeTypewriteTimer = null;
                return;
            }
            pathRow.textContent += text[idx];
            idx++;
        }, 18);
    }

    function clearPath(pathRow) {
        if (activeTypewriteTimer !== null) {
            clearInterval(activeTypewriteTimer);
            activeTypewriteTimer = null;
        }
        pathRow.classList.remove('visible');
        pathRow.textContent = '';
    }

    function initialize() {
        fileListEl = document.getElementById('file-list');
        statusEl = document.getElementById('status-text');

        if (!fileListEl || !statusEl) return false;

        FileLogic.onSlotChange(function () {
            updateSelection();
            updateStatus();
        });

        FileLogic.onSlotsUpdate(function () {
            renderSlots();
        });

        FileLogic.onBonusUpdate(function () {
            renderBonusPanel();
            updateBonusHighlights();
        });

        return true;
    }

    function updateStatus() {
        if (!statusEl) return;
        const selectedSlot = FileLogic.getSelectedSlot();

        if (selectedSlot === null) {
            statusEl.textContent = 'ファイルをクリックして選択';
        } else {
            const slots = FileLogic.getSlots();
            if (slots[selectedSlot].isEmpty) {
                statusEl.textContent = 'Slot ' + (selectedSlot + 1) + ' ─ クリックしてファイルを選択';
            } else {
                statusEl.textContent = 'Slot ' + (selectedSlot + 1) + ' を選択中';
            }
        }
    }

    function updateSelection() {
        if (!fileListEl) return;
        const selectedSlot = FileLogic.getSelectedSlot();
        const slots = FileLogic.getSlots();
        fileListEl.querySelectorAll('.file-row').forEach(function (row) {
            const slotIdx = parseInt(row.dataset.slot, 10);
            const isSelected = slotIdx === selectedSlot;
            row.classList.toggle('selected', isSelected);

            const pathRow = document.getElementById('path-row-' + slotIdx);
            if (pathRow) {
                const hasPath = !slots[slotIdx].isEmpty && slots[slotIdx].filePath;
                if (isSelected && hasPath) {
                    typewritePath(pathRow, slots[slotIdx].filePath);
                } else {
                    clearPath(pathRow);
                }
            }

            if (isSelected) {
                row.classList.remove('pulse');
                void row.offsetWidth;
                row.classList.add('pulse');
                row.addEventListener('animationend', function () {
                    row.classList.remove('pulse');
                }, { once: true });
            }
        });
    }

    function updateBonusHighlights() {
        const activeExts = FileLogic.getActiveExtensions();
        document.querySelectorAll('.bonus-entry[data-ext]').forEach(function (el) {
            const wasActive = el.classList.contains('active');
            const isActive = activeExts.has(el.dataset.ext);
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

    function renderSlots() {
        if (!fileListEl) return;
        fileListEl.innerHTML = '';

        const slots = FileLogic.getSlots();
        const prevEmpty = FileLogic.getPrevEmpty();

        slots.forEach(function (s, i) {
            const et = s.extType || 'Unknown';
            const isEmpty = s.isEmpty;
            const justLoaded = prevEmpty[i] && !isEmpty;
            const isSelected = i === FileLogic.getSelectedSlot();

            const wrap = document.createElement('div');
            wrap.className = 'file-slot';

            const row = document.createElement('div');
            row.className = 'file-row' + (isSelected ? ' selected' : '') + (justLoaded ? ' anim-in' : '');
            row.dataset.slot = i;
            row.onclick = function () { FileLogic.selectSlot(i); };

            const badge = isEmpty ? '<span></span>' :
                '<img class="ext-badge" src="' + (FileLogic.EXT_ICON[et] || FileLogic.EXT_ICON.Unknown) + '" alt="' + (FileLogic.EXT_LABEL[et] || '?') + '">';

            const bonusText = (!isEmpty && FileLogic.getBonusDesc(et)) ? FileLogic.getBonusDesc(et) : null;
            const bonus = bonusText
                ? '<span class="bonus-text has-bonus">' + bonusText + '</span>'
                : '<span class="bonus-text">—</span>';

            const iconHtml = isEmpty ?
                '<img class="file-icon" src="https://assets.game.web/images/ui/select/emp.png" alt="未選択">' :
                '<img class="file-icon" src="' + (FileLogic.EXT_ICON[et] || FileLogic.EXT_ICON.Unknown) + '" alt="' + (FileLogic.EXT_LABEL[et] || '?') + '">';

            row.innerHTML =
                '<div class="file-name-cell">' +
                    iconHtml +
                    '<span class="file-name' + (isEmpty ? ' empty' : '') + '" id="fname-' + i + '">' +
                        (isEmpty ? '─ 未選択 ─' : '') +
                    '</span>' +
                '</div>' +
                badge + bonus;

            // 新規ロード時: anim-in(280ms)完了後にタイプライター演出
            if (!isEmpty) {
                const fileName = s.fileName || '—';
                if (justLoaded) {
                    setTimeout(function () {
                        const nameEl = document.getElementById('fname-' + i);
                        if (!nameEl) return;
                        let idx = 0;
                        const timer = setInterval(function () {
                            if (idx >= fileName.length) { clearInterval(timer); return; }
                            nameEl.textContent += fileName[idx];
                            idx++;
                        }, 18);
                    }, 290);
                } else {
                    const nameEl = row.querySelector('#fname-' + i);
                    if (nameEl) nameEl.textContent = fileName;
                }
            }

            const pathRow = document.createElement('div');
            pathRow.className = 'file-path-row' + (isSelected && s.filePath ? ' visible' : '');
            pathRow.id = 'path-row-' + i;
            pathRow.textContent = s.filePath || '';

            wrap.appendChild(row);
            wrap.appendChild(pathRow);
            fileListEl.appendChild(wrap);

            prevEmpty[i] = isEmpty;
        });

        updateStatus();
        updateBonusHighlights();
    }

    function renderBonusPanel() {
        const descs = FileLogic.getBonusDescs();
        document.querySelectorAll('.bonus-entry[data-ext]').forEach(function (el) {
            const desc = descs[el.dataset.ext];
            const valEl = el.querySelector('.bonus-entry-val');
            if (valEl && desc) valEl.textContent = desc;
        });
    }

    return {
        initialize: initialize,
        renderSlots: renderSlots,
        renderBonusPanel: renderBonusPanel,
        updateSelection: updateSelection,
        updateStatus: updateStatus,
        updateBonusHighlights: updateBonusHighlights
    };
}());

window.onMessageFromGame = function (data) {
    FileLogic.onMessageFromGame(data);
};

(function () {
    if (FileView.initialize()) {
        FileView.renderSlots();
        FileLogic.requestBonusInfo();
    }
}());
