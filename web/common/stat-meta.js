'use strict';

/**
 * ステータス項目の表示情報（日本語名・アイコン・単位）。
 *
 * ファイル選択ウィンドウの拡張子ボーナス一覧と、ルール説明の装備ページの
 * 両方が同じ絵と同じ呼び名で出すために共有している。
 * キーは C++ 側が送る statId（FileSelectWindow の STAT_FIELDS）と一致させること。
 */
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
