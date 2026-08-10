#!/usr/bin/env python3
"""何も入っていない普通のブロックのテクスチャ(PNG)を生成する。

拡張子ブロックは面いっぱいにアイコンが乗っていて「壊すと何か手に入る」と分かる。
その対比として、こちらは中身を示す絵を一切置かない。
面には画像編集ソフトで透明を表すあの市松模様だけを敷く。
「ここには何も無い」を表す記号として既に知られているので、
文字を読ませずに空だと伝えられる。

下地・縁は拡張子ブロック（gen_block_textures.py）と揃える。同じ暗色の仲間に
見せておかないと、壊せる物と壊せない物の見分けが付かなくなる。

出力（assets/model/stage/ 配下）:
    BlockPlain.png … 512x512 テクスチャ

使い方（リポジトリのルートで実行）:
    python tools/gen_plain_block_texture.py
"""
import os

from PIL import Image, ImageDraw, ImageFilter

OUT_DIR = os.path.join("assets", "model", "stage")
OUT_NAME = "BlockPlain.png"

# テクスチャの一辺（他のブロックテクスチャに合わせる）
SIZE = 512

# 面の下地と縁（拡張子ブロックと同じ値）
BACKGROUND = (17, 20, 26)
BORDER_COLOR = (44, 58, 74)
BORDER_WIDTH = 6
BORDER_INSET = 10

# 市松模様。明暗の差を小さくして、模様が主役に見えないようにする
CHECKER_SIZE = 64
CHECKER_COLOR = (32, 39, 50)

# 面の中央をわずかに暗くする。全面が均一だと平らな板に見えて厚みが出ない
VIGNETTE_COLOR = (0, 0, 0, 54)
VIGNETTE_RATIO = 0.62


def make_checker():
    """透明を表す市松模様を敷いた下地を作る"""
    image = Image.new("RGBA", (SIZE, SIZE), BACKGROUND + (255,))
    draw = ImageDraw.Draw(image)

    for y in range(0, SIZE, CHECKER_SIZE):
        for x in range(0, SIZE, CHECKER_SIZE):
            if (x // CHECKER_SIZE + y // CHECKER_SIZE) % 2 == 0:
                continue
            draw.rectangle((x, y, x + CHECKER_SIZE - 1, y + CHECKER_SIZE - 1),
                           fill=CHECKER_COLOR + (255,))
    return image


def apply_vignette(image):
    """中央をうっすら暗くして面に丸みを出す"""
    layer = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)
    radius = int(SIZE * VIGNETTE_RATIO * 0.5)
    center = SIZE // 2
    draw.ellipse((center - radius, center - radius, center + radius, center + radius),
                 fill=VIGNETTE_COLOR)
    layer = layer.filter(ImageFilter.GaussianBlur(radius=SIZE * 0.10))
    return Image.alpha_composite(image, layer)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    image = apply_vignette(make_checker())

    draw = ImageDraw.Draw(image)
    draw.rectangle((BORDER_INSET, BORDER_INSET, SIZE - 1 - BORDER_INSET, SIZE - 1 - BORDER_INSET),
                   outline=BORDER_COLOR + (255,), width=BORDER_WIDTH)

    output = os.path.join(OUT_DIR, OUT_NAME)
    image.convert("RGB").save(output)
    print(f"生成: {output}")


if __name__ == "__main__":
    main()
