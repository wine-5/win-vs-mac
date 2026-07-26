'use strict';

const FILE_EXT_ICON  = {
    Executable: 'https://assets.game.web/images/ui/select/exe.png',
    Document:   'https://assets.game.web/images/ui/select/doc.png',
    Image:      'https://assets.game.web/images/ui/select/img.png',
    Audio:      'https://assets.game.web/images/ui/select/aud.png',
    SourceCode: 'https://assets.game.web/images/ui/select/src.png',
    Shortcut:   'https://assets.game.web/images/ui/select/lnk.png',
    Video:      'https://assets.game.web/images/ui/select/vid.png',
    Archive:    'https://assets.game.web/images/ui/select/arc.png',
    Unknown:    'https://assets.game.web/images/ui/select/etc.png',
};
const FILE_EXT_LABEL = {
    Executable: 'EXE', Document: 'DOC', Image: 'IMG', Audio: 'AUD',
    SourceCode: 'SRC', Shortcut: 'LNK', Video: 'VID',
    Archive: 'ARC', Unknown: 'ETC'
};
// 拡張子ボーナス一覧に出す並び順と、その行に書く対象ファイルの例。
// 並びはパラメータウィンドウのステータス順（HP→ATK→DEF→SPD→CRIT→B.SPD→B.RNG）に合わせ、
// 「どの行がどのステータスを伸ばすのか」を上から目で追えるようにする。
// 複数ステータスを伸ばす Archive と、文言が「上記以外のすべて」になる Unknown だけは末尾に置く
// ボーナスの値そのものは C++（extensionBonus.json が正）から descs で届くので持たない
const FILE_EXT_ORDER = [
    'Audio', 'Executable', 'Image', 'Document',
    'SourceCode', 'Shortcut', 'Video', 'Archive', 'Unknown'
];
// 対象の拡張子はC++（FileExtensionTypeResolverの判定表が正）から exts で届く。
// ここに持つのは、拡張子を列挙できない Unknown の文言だけ
const FILE_EXT_FALLBACK_NAME = { Unknown: '上記以外のすべて' };
// ステータスの見せ方。アイコンはパラメータウィンドウと同じ画像を使い、
// 「どの行が伸びるのか」を絵で結びつける。値そのものはC++から届く
const STAT_META = {
    hp:   { name: '体力',             icon: 'https://assets.game.web/images/ui/select/hp.png',   suffix: '' },
    atk:  { name: '攻撃力',           icon: 'https://assets.game.web/images/ui/select/atk.png',  suffix: '' },
    def:  { name: '防御力',           icon: 'https://assets.game.web/images/ui/select/def.png',  suffix: '' },
    spd:  { name: '移動速度',         icon: 'https://assets.game.web/images/ui/select/spd.png',  suffix: '' },
    rng:  { name: '攻撃範囲',         icon: 'https://assets.game.web/images/ui/select/rng.png',  suffix: '' },
    crit: { name: 'クリティカル確率', icon: 'https://assets.game.web/images/ui/select/crit.png', suffix: '%' },
    bspd: { name: '弾速',             icon: 'https://assets.game.web/images/ui/select/bspd.png', suffix: '' },
    brng: { name: '弾の飛距離',       icon: 'https://assets.game.web/images/ui/select/brng.png', suffix: '' }
};

const FILE_EXT_CLASS = {
    Executable: 'exe', Document: 'doc', Image: 'img', Audio: 'aud',
    SourceCode: 'src', Shortcut: 'lnk', Video: 'vid',
    Archive: 'arc', Unknown: 'unk'
};

const FileLogic = (function () {
    const slots = [
        { isEmpty: true, fileName: null, filePath: null, extType: null },
        { isEmpty: true, fileName: null, filePath: null, extType: null },
        { isEmpty: true, fileName: null, filePath: null, extType: null },
    ];

    const prevEmpty = [true, true, true];
    let selectedSlot = null;
    let extBonusDescs = {};
    let extBonusExtensions = {};
    let extBonusStats = {};
    // 「同一ファイル」チェックの状態。オンなら1つ選ぶだけで3枠すべてに入る
    let sameFileMode = false;
    let onSlotChangeCallback = null;
    let onBonusUpdateCallback = null;
    // 装備の更新は表示側と操作ガイドの両方が知る必要があるため、複数の購読を受け付ける
    const slotsUpdateCallbacks = [];

    function selectSlot(i) {
        selectedSlot = i;
        if (onSlotChangeCallback) {
            onSlotChangeCallback(i);
        }
        sendToGame({ type: 'slotSelected', slot: i, sameFile: sameFileMode });
    }

    /** 「同一ファイル」チェックの状態を切り替える */
    function setSameFileMode(enabled) {
        sameFileMode = enabled;
    }

    function getSelectedSlot() {
        return selectedSlot;
    }

    function getSlots() {
        return slots;
    }

    function getPrevEmpty() {
        return prevEmpty;
    }

    function updateSlots(newSlots) {
        newSlots.forEach(function (info) {
            if (info.slot >= 0 && info.slot < 3) {
                slots[info.slot] = info;
            }
        });
        slotsUpdateCallbacks.forEach(function (callback) { callback(); });
    }

    function getActiveExtensions() {
        const activeExts = new Set();
        slots.forEach(function (s) {
            if (!s.isEmpty && s.extType) activeExts.add(s.extType);
        });
        return activeExts;
    }

    function updateBonusDescs(descs, exts, stats) {
        extBonusDescs = descs;
        if (exts) extBonusExtensions = exts;
        if (stats) extBonusStats = stats;
        if (onBonusUpdateCallback) {
            onBonusUpdateCallback();
        }
    }

    function getBonusDesc(extType) {
        return extBonusDescs[extType] || null;
    }

    function getBonusDescs() {
        return extBonusDescs;
    }

    /** 種別のボーナス内訳（[{stat, value}, ...]）を返す */
    function getBonusStats(extType) {
        return extBonusStats[extType] || [];
    }

    /** 種別に属する拡張子の一覧文字列を返す（Unknownは列挙できないので固定文言） */
    function getBonusExtensions(extType) {
        return extBonusExtensions[extType] || FILE_EXT_FALLBACK_NAME[extType] || '';
    }

    function onMessageFromGame(data) {
        if (data.type === 'bonusInfo' && data.descs) {
            updateBonusDescs(data.descs, data.exts, data.bonusStats);
        } else if (data.type === 'refresh' && Array.isArray(data.slots)) {
            updateSlots(data.slots);
        }
    }

    function requestBonusInfo() {
        sendToGame({ type: 'requestBonusInfo' });
    }

    /** 現在の装備状態を要求する（ページ読み込み直後は状態が空のため） */
    function requestSlots() {
        sendToGame({ type: 'requestSlots' });
    }

    function onSlotChange(callback) {
        onSlotChangeCallback = callback;
    }

    function onBonusUpdate(callback) {
        onBonusUpdateCallback = callback;
    }

    function onSlotsUpdate(callback) {
        slotsUpdateCallbacks.push(callback);
    }

    /** 初回だけ出す操作ガイドを表示してよいかをC++へ問い合わせる */
    function requestTutorial() {
        sendToGame({ type: 'requestTutorial' });
    }

    return {
        EXT_ICON: FILE_EXT_ICON,
        EXT_LABEL: FILE_EXT_LABEL,
        EXT_CLASS: FILE_EXT_CLASS,
        EXT_ORDER: FILE_EXT_ORDER,
        getBonusExtensions: getBonusExtensions,
        getBonusStats: getBonusStats,
        STAT_META: STAT_META,
        selectSlot: selectSlot,
        setSameFileMode: setSameFileMode,
        getSelectedSlot: getSelectedSlot,
        getSlots: getSlots,
        getPrevEmpty: getPrevEmpty,
        updateSlots: updateSlots,
        getActiveExtensions: getActiveExtensions,
        updateBonusDescs: updateBonusDescs,
        getBonusDesc: getBonusDesc,
        getBonusDescs: getBonusDescs,
        onMessageFromGame: onMessageFromGame,
        requestBonusInfo: requestBonusInfo,
        requestSlots: requestSlots,
        requestTutorial: requestTutorial,
        onSlotChange: onSlotChange,
        onBonusUpdate: onBonusUpdate,
        onSlotsUpdate: onSlotsUpdate
    };
}());
