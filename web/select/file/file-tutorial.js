'use strict';

/**
 * 初回だけ出る操作ガイド。
 *
 * 「急にこの画面に来て何をすればいいのか分からない」という指摘への対処。
 * セレクト画面の各ウィンドウは別々のWebViewで互いに重ねられないため、
 * 案内はこのファイル選択ウィンドウの中で完結させ、最後にデスクトップの
 * 「ルール説明.txt」へ送り出す形にしている。
 */
const FileTutorial = (function () {
    const STEPS = [
        {
            targetSelector: '#file-list',
            title: 'まず、装備するファイルを選ぼう',
            body: 'ここに並んでいる3つの枠が装備スロットです。' +
                  '枠をクリックするとPC内のファイルを選べます。まずは1つ選んでみてください。',
            // ファイルが1つ入った時点で自動的に次へ進む（「次へ」は出さない）
            advancesOnEquip: true,
            atTop: false
        },
        {
            targetSelector: '.bonus-panel',
            title: '選んだ拡張子で能力が上がる',
            body: '装備したファイルの拡張子に応じて、キャラクターのステータスが伸びます。' +
                  'この一覧が対応表で、今装備している拡張子の行が光ります。' +
                  '伸びた数値は「パラメータ」ウィンドウで確認できます。',
            advancesOnEquip: false,
            // この段の対象は画面下端の一覧そのもの。案内を下に出すと隠してしまうので上に置く
            atTop: true
        },
        {
            targetSelector: null,
            title: '詳しい操作は「ルール説明.txt」へ',
            body: 'デスクトップにある「ルール説明.txt」をダブルクリックすると、' +
                  '操作方法とゲームの目的が読めます。' +
                  '準備ができたら右下の「ゲームを開始 →」で出撃してください。',
            advancesOnEquip: false,
            atTop: false
        }
    ];

    let tipEl = null;
    let stepEl = null;
    let titleEl = null;
    let bodyEl = null;
    let nextEl = null;
    let skipEl = null;
    let currentStep = -1;
    let isActive = false;

    function clearTarget() {
        document.querySelectorAll('.tutorial-target').forEach(function (el) {
            el.classList.remove('tutorial-target');
        });
    }

    function finish() {
        isActive = false;
        clearTarget();
        if (tipEl) tipEl.hidden = true;
    }

    function showStep(index) {
        if (index >= STEPS.length) {
            finish();
            return;
        }

        currentStep = index;
        const step = STEPS[index];

        clearTarget();
        if (step.targetSelector) {
            const target = document.querySelector(step.targetSelector);
            if (target) target.classList.add('tutorial-target');
        }

        stepEl.textContent = 'STEP ' + (index + 1) + ' / ' + STEPS.length;
        titleEl.textContent = step.title;
        bodyEl.textContent = step.body;

        // 装備で自動的に進む段では「次へ」を出さない。
        // ボタンで飛ばせてしまうと、肝心のファイル選択をせずに読み終えてしまう
        nextEl.hidden = step.advancesOnEquip;
        nextEl.textContent = (index === STEPS.length - 1) ? '閉じる' : '次へ →';
        skipEl.hidden = (index === STEPS.length - 1);

        tipEl.classList.toggle('at-top', step.atTop === true);
        tipEl.hidden = false;
    }

    /** 装備が変わったときに呼ばれる。ファイルが1つでも入っていれば次の段へ進む */
    function onSlotsUpdated() {
        if (!isActive) return;

        const step = STEPS[currentStep];
        if (!step || !step.advancesOnEquip) return;

        const hasEquipped = FileLogic.getSlots().some(function (s) { return !s.isEmpty; });
        if (hasEquipped) showStep(currentStep + 1);
    }

    function start() {
        if (isActive) return;

        tipEl = document.getElementById('tutorial-tip');
        stepEl = document.getElementById('tutorial-step');
        titleEl = document.getElementById('tutorial-title');
        bodyEl = document.getElementById('tutorial-body');
        nextEl = document.getElementById('tutorial-next');
        skipEl = document.getElementById('tutorial-skip');
        if (!tipEl || !stepEl || !titleEl || !bodyEl || !nextEl || !skipEl) return;

        nextEl.onclick = function () { showStep(currentStep + 1); };
        skipEl.onclick = finish;

        isActive = true;
        showStep(0);
    }

    function onMessageFromGame(data) {
        if (data.type === 'tutorial' && data.show) start();
    }

    return {
        onMessageFromGame: onMessageFromGame,
        onSlotsUpdated: onSlotsUpdated
    };
}());

(function () {
    // 表示側の購読より後に登録される。どちらも呼ばれるので順番は問わない
    FileLogic.onSlotsUpdate(FileTutorial.onSlotsUpdated);
    FileLogic.requestTutorial();
}());
