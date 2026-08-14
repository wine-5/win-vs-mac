#!/usr/bin/env python3
"""床テクスチャに擬似ノーマルマップの陰影を焼き込み、平らな板に凹凸感を出す。

なぜ焼き込みなのか:
    DxLib には MV1SetMaterialNormalMapTexture があり、MQOのマテリアルに
    bump("xxx.png") と書けば法線マップとして読み込まれる（MV1GetTextureNum=2、
    MV1GetMaterialNormalMapTexture=1 になることは実機で確認済み）。
    しかし MQO 由来のメッシュには接線情報が無いため、**描画には反映されない**。
    法線マップの強さを25倍にしても絵が1ピクセルも変わらないことを確認した。

    本作の平行光は InGame.cpp で固定なので、法線マップで得られるはずの陰影は
    テクスチャ側へ焼いてしまえば見た目が同じになる。実行時コストはゼロで、
    モデル・シェーダ・描画コードのどれにも手を入れずに済む。

やること:
    * 元テクスチャの輝度をハイトマップとみなし、Sobel（周期境界）で法線を作る
    * 平行光との内積を「平らな面との比」に直し、元の色へ掛ける
    * 継ぎ目が出ないよう、勾配は必ず wrap で計算する

床の天面のタンジェント空間（FactoryInitializer.cpp のUVの張り方に合わせている）:
    U軸 = ワールド +X / V軸 = ワールド -Z / N軸 = ワールド +Y

使い方（リポジトリのルートで実行）:
    # 焼き込み前の絵は git 履歴から取り出す（PowerShellの > はPNGを壊すので使わないこと）
    git cat-file blob <焼き込み前のcommit>:assets/model/stage/FloorDesktop.png > base.png   # bash
    python tools/bake_floor_shading.py --src base.png --name FloorDesktop

    # 効き具合を変える（strengthで凹凸の急峻さ、amountで陰影の濃さ）
    python tools/bake_floor_shading.py --src ... --name FloorDesktop --strength 8 --amount 0.5
"""
import argparse
import os

import numpy as np
from PIL import Image, ImageFilter

OUT_DIR = os.path.join("assets", "model", "stage")

# InGame.cpp の平行光の向き（光が進む向き）。面から光源へ向かうベクトルはこの逆。
LIGHT_TRAVEL = (-0.3, -1.0, 0.4)

DEFAULT_STRENGTH = 6.0
DEFAULT_BLUR = 0.8
DEFAULT_AMOUNT = 0.7


def _wrap_shift(a, dy, dx):
    """周期境界でずらす。タイリングする床なので継ぎ目を作らないために使う。"""
    return np.roll(np.roll(a, dy, axis=0), dx, axis=1)


def _sobel(height):
    """ハイトマップから u/v 方向の勾配を返す（wrap）。"""
    gu = ((_wrap_shift(height, -1, -1) + 2 * _wrap_shift(height, 0, -1) + _wrap_shift(height, 1, -1))
          - (_wrap_shift(height, -1, 1) + 2 * _wrap_shift(height, 0, 1) + _wrap_shift(height, 1, 1))) / 4.0
    gv = ((_wrap_shift(height, -1, -1) + 2 * _wrap_shift(height, -1, 0) + _wrap_shift(height, -1, 1))
          - (_wrap_shift(height, 1, -1) + 2 * _wrap_shift(height, 1, 0) + _wrap_shift(height, 1, 1))) / 4.0
    return gu, gv


def bake(src_path, dst_path, strength, blur, amount):
    """1枚のテクスチャへ陰影を焼き込んで保存する。"""
    with Image.open(src_path) as img:
        rgb = np.asarray(img.convert("RGB"), dtype=np.float32) / 255.0
        gray = img.convert("L")
        if blur > 0:
            gray = gray.filter(ImageFilter.GaussianBlur(blur))
        height = np.asarray(gray, dtype=np.float32) / 255.0

    gu, gv = _sobel(height)
    nx, ny, nz = gu * strength, gv * strength, np.ones_like(gu)
    length = np.sqrt(nx * nx + ny * ny + nz * nz)
    nx, ny, nz = nx / length, ny / length, nz / length

    # 面 -> 光源のベクトルをタンジェント空間へ移す
    to_light = np.array([-c for c in LIGHT_TRAVEL], dtype=np.float32)
    tangent_light = np.array([to_light[0], -to_light[2], to_light[1]], dtype=np.float32)
    tangent_light /= np.linalg.norm(tangent_light)
    flat = float(tangent_light[2])  # 平らな面（法線 = (0,0,1)）での明るさ

    lambert = nx * tangent_light[0] + ny * tangent_light[1] + nz * tangent_light[2]
    factor = 1.0 + (np.clip(lambert / flat, 0.0, 2.0) - 1.0) * amount

    out = np.clip(rgb * factor[..., None], 0.0, 1.0)
    Image.fromarray((out * 255.0 + 0.5).astype(np.uint8), "RGB").save(dst_path, "PNG", optimize=True)
    print("baked %s -> %s (strength=%.1f blur=%.1f amount=%.2f flat=%.3f)"
          % (os.path.basename(src_path), dst_path, strength, blur, amount, flat))


def main():
    parser = argparse.ArgumentParser(description="床テクスチャへ擬似ノーマルマップの陰影を焼き込む")
    parser.add_argument("--src", required=True, help="焼き込み前のPNG")
    parser.add_argument("--name", required=True, help="出力名（assets/model/stage/<name>.png へ書き出す）")
    parser.add_argument("--strength", type=float, default=DEFAULT_STRENGTH, help="凹凸の急峻さ")
    parser.add_argument("--blur", type=float, default=DEFAULT_BLUR, help="ハイトマップのぼかし半径")
    parser.add_argument("--amount", type=float, default=DEFAULT_AMOUNT, help="陰影の濃さ（0で無効、1で全開）")
    args = parser.parse_args()

    dst = os.path.join(OUT_DIR, args.name + ".png")
    bake(args.src, dst, args.strength, args.blur, args.amount)


if __name__ == "__main__":
    main()
