'use strict';

/**
 * 操作ページの、キーボードとコントローラーの図の切り替え。
 *
 * 操作の中身は図（画像）側に描き込んであるので、ここがするのは
 * 「どちらの図を出すか」の付け替えだけ。読んだ位置などは持たない。
 */
(function () {
    const switchItems = document.querySelectorAll('.device-switch-item[data-device]');
    const images = document.querySelectorAll('.device-figure .device-image[data-device]');

    function activate(device) {
        // 押された側と出ている図を同じ属性で選ぶ。両者を別々に持つと、
        // 図を足したときに片方だけ直して食い違う
        switchItems.forEach(function (item) {
            item.classList.toggle('is-active', item.dataset.device === device);
        });
        images.forEach(function (image) {
            image.classList.toggle('is-active', image.dataset.device === device);
        });
    }

    switchItems.forEach(function (item) {
        item.onclick = function () { activate(item.dataset.device); };
    });
}());
