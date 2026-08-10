#!/usr/bin/env python3
"""RAMブロック（壊すと装備できる枠が1つ増えるブロック）のテクスチャ(PNG)を生成する。

メモリモジュールの基板をそのまま面に貼る。緑の基板・金色の端子・黒いチップという
並びは実物を見たことがある人なら一目で分かるので、文字を置かずに何のブロックか伝わる。

他のブロックが暗い青灰なのに対してこれだけ緑にしてあるのは、
道中で見つけたときに「あれは違うものだ」と遠くから気付かせるため。
壊すと枠が増える一点物で、拡張子ブロックのように何度も出るものではない。

出力（assets/model/stage/ 配下）:
    BlockRam.png … 512x512 テクスチャ

使い方（リポジトリのルートで実行）:
    python tools/gen_ram_block_texture.py
"""
import os

from PIL import Image, ImageDraw, ImageFilter

OUT_DIR = os.path.join("assets", "model", "stage")
OUT_NAME = "BlockRam.png"

# テクスチャの一辺（他のブロックテクスチャに合わせる）
SIZE = 512

# 基板の緑。彩度を上げすぎると他のHUDの緑（強化を示す色）と意味が混ざるので抑える
BOARD_COLOR = (26, 74, 52)
BOARD_EDGE_COLOR = (46, 112, 80)
BORDER_WIDTH = 6
BORDER_INSET = 10

# 端子（基板の下端に並ぶ金色の接点）
PIN_COLOR = (198, 162, 74)
PIN_AREA_TOP = 404
PIN_HEIGHT = 74
PIN_WIDTH = 14
PIN_GAP = 10
PIN_MARGIN_X = 30

# 端子を左右に分ける切り欠き。実物のDIMMにある位置合わせの溝
NOTCH_WIDTH = 26
NOTCH_CENTER_RATIO = 0.42  # 中央からわずかに左（実物と同じく非対称）

# チップ（黒い四角）。4つ並べるとメモリモジュールらしくなる
CHIP_COLOR = (18, 22, 28)
CHIP_EDGE_COLOR = (54, 62, 72)
CHIP_TOP = 150
CHIP_HEIGHT = 190
CHIP_WIDTH = 96
CHIP_GAP = 20

# 基板の上端に走る配線。細い線を数本引くだけで「回路」に見える
TRACE_COLOR = (58, 138, 100)
TRACE_TOP = 70
TRACE_GAP = 16
TRACE_COUNT = 4

# 面の中央をわずかに明るくする。均一だと平らな板に見えて厚みが出ない
GLOW_COLOR = (74, 222, 128, 40)
GLOW_RATIO = 0.72


def draw_pins(draw):
    """基板下端の端子列を描く（切り欠きで左右に分かれる）"""
    notch_center = int(SIZE * NOTCH_CENTER_RATIO)
    notch_left = notch_center - NOTCH_WIDTH // 2
    notch_right = notch_center + NOTCH_WIDTH // 2

    x = PIN_MARGIN_X
    while x + PIN_WIDTH < SIZE - PIN_MARGIN_X:
        if not (notch_left <= x <= notch_right):
            draw.rectangle((x, PIN_AREA_TOP, x + PIN_WIDTH, PIN_AREA_TOP + PIN_HEIGHT),
                           fill=PIN_COLOR)
        x += PIN_WIDTH + PIN_GAP


def draw_chips(draw):
    """基板の中央に並ぶチップを描く"""
    total = CHIP_WIDTH * 4 + CHIP_GAP * 3
    left = (SIZE - total) // 2

    for i in range(4):
        x = left + i * (CHIP_WIDTH + CHIP_GAP)
        draw.rectangle((x, CHIP_TOP, x + CHIP_WIDTH, CHIP_TOP + CHIP_HEIGHT),
                       fill=CHIP_COLOR, outline=CHIP_EDGE_COLOR, width=2)


def draw_traces(draw):
    """基板上端の配線を描く"""
    for i in range(TRACE_COUNT):
        y = TRACE_TOP + i * TRACE_GAP
        # 左右で長さを変える。すべて同じ長さだと模様に見えて回路に見えない
        right = SIZE - PIN_MARGIN_X - (i % 2) * 60
        draw.line((PIN_MARGIN_X, y, right, y), fill=TRACE_COLOR, width=3)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    image = Image.new("RGBA", (SIZE, SIZE), BOARD_COLOR + (255,))
    draw = ImageDraw.Draw(image)

    draw_traces(draw)
    draw_chips(draw)
    draw_pins(draw)

    glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    radius = int(SIZE * GLOW_RATIO * 0.5)
    center = SIZE // 2
    glow_draw.ellipse((center - radius, center - radius, center + radius, center + radius),
                      fill=GLOW_COLOR)
    image = Image.alpha_composite(image, glow.filter(ImageFilter.GaussianBlur(radius=SIZE * 0.10)))

    draw = ImageDraw.Draw(image)
    draw.rectangle((BORDER_INSET, BORDER_INSET, SIZE - 1 - BORDER_INSET, SIZE - 1 - BORDER_INSET),
                   outline=BOARD_EDGE_COLOR, width=BORDER_WIDTH)

    output = os.path.join(OUT_DIR, OUT_NAME)
    image.convert("RGB").save(output)
    print(f"生成: {output}")


if __name__ == "__main__":
    main()
