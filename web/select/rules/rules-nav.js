'use strict';

/**
 * メニューによるページ切り替え。
 *
 * 説明は「順に読ませる導線」ではなく「あとで引く分冊」として分けている。
 * そのため切り替えは表示の付け替えだけで、読んだ位置などは持たない。
 */
(function () {
    const menuItems = document.querySelectorAll('.menubar .menu-item[data-page]');
    const pages = document.querySelectorAll('#content .page');
    const content = document.getElementById('content');

    function activate(pageId) {
        // 選択中の項目は目立たせる。メニューしか入口が無いので、
        // 今どれを読んでいるのかを示すものが他に無い
        menuItems.forEach(function (item) {
            item.classList.toggle('is-active', item.dataset.page === pageId);
        });
        pages.forEach(function (page) {
            page.classList.toggle('is-active', page.id === 'page-' + pageId);
        });
        // 前のページで下まで送っていると、切り替えた先も途中から表示されて
        // 見出しを読み飛ばしてしまうため、毎回先頭へ戻す
        if (content) content.scrollTop = 0;
    }

    menuItems.forEach(function (item) {
        item.onclick = function () { activate(item.dataset.page); };
        item.dataset.padFocus = '';
    });
}());
