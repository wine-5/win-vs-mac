'use strict';

/**
 * メモ帳風タブの切り替え。
 *
 * 説明は「順に読ませる導線」ではなく「あとで引く分冊」として分けている。
 * そのため切り替えは表示の付け替えだけで、読んだ位置などは持たない。
 */
(function () {
    const tabs = document.querySelectorAll('#tabbar .tab');
    const pages = document.querySelectorAll('#content .page');
    const content = document.getElementById('content');

    function activate(pageId) {
        tabs.forEach(function (tab) {
            tab.classList.toggle('is-active', tab.dataset.page === pageId);
        });
        pages.forEach(function (page) {
            page.classList.toggle('is-active', page.id === 'page-' + pageId);
        });
        // 前のタブで下まで送っていると、切り替えた先も途中から表示されて
        // 見出しを読み飛ばしてしまうため、毎回先頭へ戻す
        if (content) content.scrollTop = 0;
    }

    tabs.forEach(function (tab) {
        tab.onclick = function () { activate(tab.dataset.page); };
    });
}());
