'use strict';

const FILE_EXT_ICON  = {
    Executable: 'https://assets.game.web/images/ui/select/exe.png',
    Document:   'https://assets.game.web/images/ui/select/doc.png',
    Image:      'https://assets.game.web/images/ui/select/img.png',
    Audio:      'https://assets.game.web/images/ui/select/aud.png',
    Archive:    'https://assets.game.web/images/ui/select/arc.png',
    Unknown:    'https://assets.game.web/images/ui/select/gameicon.png',
};
const FILE_EXT_LABEL = { Executable: 'EXE', Document: 'DOC', Image: 'IMG', Audio: 'AUD', Archive: 'ARC', Unknown: '???' };
const FILE_EXT_CLASS = { Executable: 'exe', Document: 'doc', Image: 'img', Audio: 'aud', Archive: 'arc', Unknown: 'unk' };

const FileLogic = (function () {
    const slots = [
        { isEmpty: true, fileName: null, filePath: null, extType: null },
        { isEmpty: true, fileName: null, filePath: null, extType: null },
        { isEmpty: true, fileName: null, filePath: null, extType: null },
    ];

    const prevEmpty = [true, true, true];
    let selectedSlot = null;
    let extBonusDescs = {};
    let onSlotChangeCallback = null;
    let onBonusUpdateCallback = null;
    let onSlotsUpdateCallback = null;

    function selectSlot(i) {
        selectedSlot = i;
        if (onSlotChangeCallback) {
            onSlotChangeCallback(i);
        }
        sendToGame({ type: 'slotSelected', slot: i });
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
        if (onSlotsUpdateCallback) {
            onSlotsUpdateCallback();
        }
    }

    function getActiveExtensions() {
        const activeExts = new Set();
        slots.forEach(function (s) {
            if (!s.isEmpty && s.extType) activeExts.add(s.extType);
        });
        return activeExts;
    }

    function updateBonusDescs(descs) {
        extBonusDescs = descs;
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

    function onMessageFromGame(data) {
        if (data.type === 'bonusInfo' && data.descs) {
            updateBonusDescs(data.descs);
        } else if (data.type === 'refresh' && Array.isArray(data.slots)) {
            updateSlots(data.slots);
        }
    }

    function requestBonusInfo() {
        sendToGame({ type: 'requestBonusInfo' });
    }

    function onSlotChange(callback) {
        onSlotChangeCallback = callback;
    }

    function onBonusUpdate(callback) {
        onBonusUpdateCallback = callback;
    }

    function onSlotsUpdate(callback) {
        onSlotsUpdateCallback = callback;
    }

    return {
        EXT_ICON: FILE_EXT_ICON,
        EXT_LABEL: FILE_EXT_LABEL,
        EXT_CLASS: FILE_EXT_CLASS,
        selectSlot: selectSlot,
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
        onSlotChange: onSlotChange,
        onBonusUpdate: onBonusUpdate,
        onSlotsUpdate: onSlotsUpdate
    };
}());
