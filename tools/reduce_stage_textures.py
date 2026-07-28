#!/usr/bin/env python3
"""ステージ配置物のテクスチャPNGを実機向けに縮小・軽量化する。

外部（ChatGPT等）で作ったテクスチャは 1024〜1254px 程度で出てくるが、実機では
**512px 運用**にしている。DxLib はテクスチャ1枚につき生データの約3倍を確保するため、
1024→512 の縮小で1枚あたり約9.7MB減る（15枚で約136MB削減した実績がある）。

やること:
    * 指定サイズ（既定512）へ正方形リサイズ（LANCZOS）
    * RGBへ変換（アルファは実機で使わないので落とす）
    * PNGを最適化して保存

使い方（リポジトリのルートで実行）:
    # 外部で作ったPNGを取り込む（コピー＋縮小して所定の名前で置く）
    python tools/reduce_stage_textures.py --src "C:/Users/xxx/Downloads/foo.png" --name FloorMemory

    # すでに assets/model/stage/ にあるPNGを一括で512pxへ揃える
    python tools/reduce_stage_textures.py --all
"""
import argparse
import glob
import os

from PIL import Image

OUT_DIR = os.path.join("assets", "model", "stage")
DEFAULT_SIZE = 512


def reduce_image(src_path, dst_path, size):
    """1枚のPNGを指定サイズの正方形RGBへ縮小して保存する。"""
    with Image.open(src_path) as img:
        before = img.size
        out = img.convert("RGB")
        if out.size != (size, size):
            out = out.resize((size, size), Image.LANCZOS)
        out.save(dst_path, "PNG", optimize=True)
    after_kb = os.path.getsize(dst_path) // 1024
    print("reduced %s %s -> %dx%d (%dKB) : %s"
          % (os.path.basename(src_path), before, size, size, after_kb, dst_path))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--src", help="取り込む元PNGのパス（外部で生成したもの）")
    parser.add_argument("--name", help="出力名（拡張子なし。例: FloorMemory）")
    parser.add_argument("--all", action="store_true",
                        help="assets/model/stage/ の全PNGを対象にする")
    parser.add_argument("--size", type=int, default=DEFAULT_SIZE,
                        help="出力サイズ（既定 %d）" % DEFAULT_SIZE)
    args = parser.parse_args()

    os.makedirs(OUT_DIR, exist_ok=True)

    if args.all:
        for path in sorted(glob.glob(os.path.join(OUT_DIR, "*.png"))):
            reduce_image(path, path, args.size)
        return

    if not args.src or not args.name:
        parser.error("--src と --name を両方指定するか、--all を指定してください")

    dst = os.path.join(OUT_DIR, args.name + ".png")
    reduce_image(args.src, dst, args.size)


if __name__ == "__main__":
    main()
