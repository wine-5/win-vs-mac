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

        // 「同一ファイル」チェック。オンにすると1つ選ぶだけで3枠すべてに同じものが入る
        const sameFileCheck = document.getElementById('same-file-check');
        if (sameFileCheck) {
            sameFileCheck.addEventListener('change', function () {
                FileLogic.setSameFileMode(sameFileCheck.checked);
            });
        }

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

        // 「同一ファイル」で3枠まとめて埋まったとき、全部が同時に現れると
        // 何が起きたのか読み取れない。今回新しく埋まった行だけを数えて、
        // 上から順に少しずつ遅らせて出す
        const SLOT_STAGGER_MS = 260;
        let loadedOrder = 0;

        slots.forEach(function (s, i) {
            const et = s.extType || 'Unknown';
            const isEmpty = s.isEmpty;
            const justLoaded = prevEmpty[i] && !isEmpty;
            const isSelected = i === FileLogic.getSelectedSlot();

            const wrap = document.createElement('div');
            wrap.className = 'file-slot';

            // 今回埋まった行の中での順番。上の行ほど先に現れる
            const appearDelayMs = justLoaded ? loadedOrder * SLOT_STAGGER_MS : 0;
            if (justLoaded) loadedOrder++;

            const row = document.createElement('div');
            // 未選択の行は is-empty を付けてCSS側の誘導アニメーションを走らせる
            row.className = 'file-row' + (isSelected ? ' selected' : '') +
                (justLoaded ? ' anim-in' : '') + (isEmpty ? ' is-empty' : '');
            // 遅延中も「出現前」の見た目を保つため、CSS側で animation-fill-mode: backwards を指定している
            if (appearDelayMs > 0)
                row.style.animationDelay = appearDelayMs + 'ms';
            row.dataset.slot = i;
            row.onclick = function () { FileLogic.selectSlot(i); };

            // 下の一覧と同じアイコン表記にする。装備中のスロットこそ一番見る場所なので、
            // 一覧より読み取りにくい略称テキストのままにしない。
            // 名前はアイコンで分かるので出さず、アイコンと数値だけを右端に揃える
            const bonus = isEmpty
                ? '<span class="bonus-text">—</span>'
                : buildStatsHtml(FileLogic.getBonusStats(et), FileLogic.getBonusDesc(et) || '—', true);

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
                bonus +
                // 装備中の行だけ、外周を光の粒が回り続ける（起動中であることの表現）
                (isEmpty ? '' : buildOrbitDots());

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
                    }, 290 + appearDelayMs);
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

    /**
     * 装備中の行の外周を回る光の粒を組み立てる
     *
     * インゲームの装備スロットに合わせ、2列の粒が向かい合って周回し、
     * 後続ほど小さく淡くして尾を引かせる。値の意味はCSSの .orbit-dot 側と対
     */
    function buildOrbitDots() {
        const PERIOD = 3.2;      // 一周にかける秒数（CSSのanimationと合わせる）
        const COMET_COUNT = 2;   // 同時に回る列の数
        const TRAIL_COUNT = 8;   // 1列あたりの粒の数
        const TRAIL_SPACING = 0.035; // 粒どうしの間隔（一周を1.0とした割合）
        const HEAD_SIZE = 6;     // 先頭の粒の直径（px）

        let html = '';
        for (let comet = 0; comet < COMET_COUNT; comet++) {
            for (let i = 0; i < TRAIL_COUNT; i++) {
                const fade = 1 - i / TRAIL_COUNT;
                // 負の遅延で「すでに進んだ状態」から始める。列は一周を等分した位置へずらす
                const delay = -(comet * PERIOD / COMET_COUNT + i * TRAIL_SPACING * PERIOD);
                const size = (HEAD_SIZE * fade).toFixed(1);
                html += '<span class="orbit-dot" style="' +
                    'animation-delay:' + delay.toFixed(3) + 's;' +
                    'width:' + size + 'px;height:' + size + 'px;' +
                    'opacity:' + (fade * fade).toFixed(2) + '"></span>';
            }
        }
        return html;
    }

    /**
     * ボーナス内訳をアイコン付きで組み立てる
     *
     * 略称だけではパラメータ画面のどの行が伸びるのか結びつかないため、
     * 向こうと同じアイコンを並べる。項目が多い行（アーカイブ）は
     * 日本語名まで出すと折り返してしまうので、アイコンと数値だけにする
     * @param stats [{ stat, value }, ...]
     * @param fallbackText 内訳が届いていないときに出す短い説明文
     * @param forceCompact 項目数によらず名前を省く（スロット行はアイコンだけで足りるため）
     */
    function buildStatsHtml(stats, fallbackText, forceCompact) {
        if (!stats || stats.length === 0)
            return '<span class="bonus-entry-val multi">' + (fallbackText || '') + '</span>';

        const showName = !forceCompact && stats.length <= 2;
        const parts = stats.map(function (s) {
            const meta = FileLogic.STAT_META[s.stat];
            if (!meta) return '';
            const value = Number.isInteger(s.value) ? s.value : Math.round(s.value * 10) / 10;
            return '<span class="bonus-stat">' +
                '<img class="bonus-stat-icon" src="' + meta.icon + '" alt="' + meta.name + '">' +
                (showName ? '<span class="bonus-stat-name">' + meta.name + '</span>' : '') +
                '<span class="bonus-stat-val">+' + value + meta.suffix + '</span>' +
                '</span>';
        });
        return '<span class="bonus-entry-val bonus-stat-list' + (showName ? '' : ' compact') + '">' +
            parts.join('') + '</span>';
    }

    /**
     * 拡張子ボーナス一覧を組み立てる。
     * 行そのものを descs（C++が extensionBonus.json から生成）で作るので、
     * 拡張子を増やしても値を変えてもHTMLを触らずに追従する
     */
    function renderBonusPanel() {
        const listEl = document.getElementById('bonus-list');
        if (!listEl) return;

        const descs = FileLogic.getBonusDescs();
        listEl.innerHTML = '';

        FileLogic.EXT_ORDER.forEach(function (ext) {
            const desc = descs[ext];
            // ボーナスが1つも設定されていない拡張子は行ごと出さない
            if (!desc) return;

            const entry = document.createElement('div');
            entry.className = 'bonus-entry';
            entry.dataset.ext = ext;
            entry.innerHTML =
                '<img class="ext-badge" src="' + (FileLogic.EXT_ICON[ext] || FileLogic.EXT_ICON.Unknown) +
                    '" alt="' + (FileLogic.EXT_LABEL[ext] || '?') + '">' +
                '<span class="bonus-entry-name">' + FileLogic.getBonusExtensions(ext) + '</span>' +
                buildStatsHtml(FileLogic.getBonusStats(ext), desc);
            listEl.appendChild(entry);
        });

        updateBonusHighlights();
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
    // HARDでは配色を警告色へ切り替える（common.jsの共通処理）
    applyDifficultyTheme(data);
    FileLogic.onMessageFromGame(data);
    if (typeof FileTutorial !== 'undefined')
        FileTutorial.onMessageFromGame(data);
};

(function () {
    if (FileView.initialize()) {
        FileView.renderSlots();
        FileLogic.requestBonusInfo();
        FileLogic.requestSlots();
    }
}());
