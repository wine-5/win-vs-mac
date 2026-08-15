#!/usr/bin/env python3
"""テクスチャの明暗を高さとみなして法線マップ（ノーマルマップ）を作る。

平らな板に凹凸を持たせるための絵で、DxLib は MQO のマテリアルに
bump("xxx_normal.png") と書いておくと法線マップとして読み込む。
陰影を焼き込む方式と違い、光の向きやカメラの動きに反応して陰影が変わる。

【重要】法線マップは背面カリングを切ったメッシュ（DX_CULLING_NONE）には効かない。
シャドウマップと同じ制約で、DxLib からは裏面に見えているため。
配置物モデルの巻き方向は tools/gen_stage_models.py 側で正しくしてある。

やること:
    * 元テクスチャの輝度をハイトマップとみなす
    * Sobel（周期境界）で傾きを求め、接空間の法線へ直す
    * RGBへ詰めて <元の名前>_normal.png として保存する

タイリングする床・壁を想定しているため、勾配は必ず wrap で計算する
（端を特別扱いすると継ぎ目に線が出る）。

使い方（リポジトリのルートで実行）:
    # assets/model/stage/FloorDesktop.png から FloorDesktop_normal.png を作る
    python tools/gen_normal_map.py --name FloorDesktop

    # 凹凸の強さを変える（strengthが大きいほど急峻、blurが大きいほど滑らか）
    python tools/gen_normal_map.py --name FloorApple --strength 12 --blur 1.4

    # 置いたあとは MQO を作り直すと bump 行が入る
    python tools/gen_stage_models.py FloorDesktop
"""
import argparse
import os

import numpy as np
from PIL import Image, ImageFilter

OUT_DIR = os.path.join("assets", "model", "stage")
NORMAL_SUFFIX = "_normal"

DEFAULT_STRENGTH = 20.0
DEFAULT_BLUR = 0.6


def _wrap_shift(a, dy, dx):
    """周期境界でずらす。タイリングする絵なので継ぎ目を作らないために使う。"""
    return np.roll(np.roll(a, dy, axis=0), dx, axis=1)


def make_normal(src_path, dst_path, strength, blur):
    """1枚のテクスチャから法線マップを作って保存する。"""
    with Image.open(src_path) as img:
        gray = img.convert("L")
        if blur > 0:
            gray = gray.filter(ImageFilter.GaussianBlur(blur))
        height = np.asarray(gray, dtype=np.float32) / 255.0

    # Sobel（wrap）でU/V方向の傾きを求める
    gu = ((_wrap_shift(height, -1, -1) + 2 * _wrap_shift(height, 0, -1) + _wrap_shift(height, 1, -1))
          - (_wrap_shift(height, -1, 1) + 2 * _wrap_shift(height, 0, 1) + _wrap_shift(height, 1, 1))) / 4.0
    gv = ((_wrap_shift(height, -1, -1) + 2 * _wrap_shift(height, -1, 0) + _wrap_shift(height, -1, 1))
          - (_wrap_shift(height, 1, -1) + 2 * _wrap_shift(height, 1, 0) + _wrap_shift(height, 1, 1))) / 4.0

    nx, ny, nz = gu * strength, gv * strength, np.ones_like(gu)
    length = np.sqrt(nx * nx + ny * ny + nz * nz)
    nx, ny, nz = nx / length, ny / length, nz / length

    # -1〜1 を 0〜255 へ詰める（法線マップが青紫に見えるのはZ成分が常に正のため）
    rgb = np.clip(np.stack([nx, ny, nz], axis=-1) * 0.5 + 0.5, 0.0, 1.0)
    Image.fromarray((rgb * 255.0 + 0.5).astype(np.uint8), "RGB").save(dst_path, "PNG", optimize=True)

    size_kb = os.path.getsize(dst_path) // 1024
    print("generated %s (strength=%.1f blur=%.1f, %dKB)"
          % (dst_path, strength, blur, size_kb))


def main():
    parser = argparse.ArgumentParser(description="テクスチャから法線マップを作る")
    parser.add_argument("--name", required=True,
                        help="assets/model/stage/<name>.png を元にする（拡張子なしの名前）")
    parser.add_argument("--strength", type=float, default=DEFAULT_STRENGTH, help="凹凸の急峻さ")
    parser.add_argument("--blur", type=float, default=DEFAULT_BLUR, help="ハイトマップのぼかし半径")
    args = parser.parse_args()

    src = os.path.join(OUT_DIR, args.name + ".png")
    if not os.path.exists(src):
        print("[error] 元テクスチャが見つかりません:", src)
        return

    dst = os.path.join(OUT_DIR, args.name + NORMAL_SUFFIX + ".png")
    make_normal(src, dst, args.strength, args.blur)


if __name__ == "__main__":
    main()
