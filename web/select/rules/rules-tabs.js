'use strict';

/**
 * メモ帳風タブの切り替え。
 *
 * 説明は「順に読ませる導線」ではなく「あとで引く分冊」として分けている。
 * そのため切り替えは表示の付け替えだけで、読んだ位置などは持たない。
 */
(function () {
    const tabs = document.querySelectorAll('#tabbar .tab');
    // メニューもタブと同じページへ移す。名前が違うだけの入口が2つある状態にして、
    // 「ファイル名で探す人」と「読みたい項目で探す人」の両方が辿り着けるようにする
    const menuItems = document.querySelectorAll('.menubar .menu-item[data-page]');
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

    menuItems.forEach(function (item) {
        item.onclick = function () { activate(item.dataset.page); };
    });
}());
