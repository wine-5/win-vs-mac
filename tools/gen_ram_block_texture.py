#!/usr/bin/env python3
"""RAMブロック（壊すと拡張子を挿せる枠が1つ増えるブロック）のテクスチャ(PNG)を生成する。

面に描くのは「枠が1つ増える」という変化そのもの。

      [  +  ]  +1
          ↑
    [DOC][IMG][AUD]

いま埋まっているマスを下に置き、その上へ矢印を向けて破線の空きマスを足す。
下から上へ伸ばすのは、増える・積み上がるという向きが上だから。
横一列ではなく縦に積むのは、正方形の面では縦のほうが空きマスを大きく取れるため。
マス目を4つ並べるだけだと「もともとそういう絵」なのか「増える」のかが読めないため、
矢印を挟んで変化として見せる。

メモリモジュールの基板を描く案もあったが、実物を知っている人にも
「メモリだ」までしか伝わらず、それが装備枠の話だとは繋がらない。
インベントリで毎回見ているマス目と拡張子アイコンをそのまま面へ持ってくれば、
既に持っている知識だけで「あの枠が増えるのだ」と読める。

文字（「所持上限+1」「CAPACITY UP」など）は入れない。
このブロックを見るのは数メートル離れた3D空間からで、語は潰れて読めない。
数字の +1 だけは記号として距離に耐えるので残す。

矢印と＋を黄色にしているのは、HUDが「強化されている」を黄色で示しているため。
色の意味を画面ごとにずらさない。

出力（assets/model/stage/ 配下）:
    BlockRam.png … 512x512 テクスチャ

使い方（リポジトリのルートで実行）:
    python tools/gen_ram_block_texture.py
"""
import os

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ICON_DIR = os.path.join("assets", "images", "ui", "ingame", "ext")
OUT_DIR = os.path.join("assets", "model", "stage")
OUT_NAME = "BlockRam.png"

# テクスチャの一辺（他のブロックテクスチャに合わせる）
SIZE = 512

# 下地の緑（基板）。彩度を上げすぎるとHUDの緑（強化を示す色）と意味が混ざるので抑える
BOARD_COLOR = (24, 66, 48)
BOARD_EDGE_COLOR = (46, 112, 80)
BORDER_WIDTH = 6
BORDER_INSET = 10

# 埋まっているマス（上に3つ並ぶ）
FILLED_ICONS = ["doc", "img", "aud"]
FILLED_SIZE = 116
FILLED_GAP = 12
FILLED_FILL = (16, 40, 30)
FILLED_BORDER = (70, 140, 105)
ICON_RATIO = 0.72 # マスに対するアイコンの大きさ
ROW_TOP = 344     # マス列の上端（面の下側に置く）

# 矢印（変化を示す）。下から上へ向ける
ARROW_HEIGHT = 56
ARROW_THICKNESS = 18
ARROW_HEAD = 34
ARROW_TOP = 270
ARROW_COLOR = (255, 200, 61)

# 増える枠（破線の空きマス）。埋まっているマスより一回り大きく描く
EMPTY_SIZE = 208
EMPTY_TOP = 46
EMPTY_BORDER = (255, 200, 61)
EMPTY_DASH = 20
EMPTY_GAP = 13
EMPTY_WIDTH = 6

# 空きマスに重ねる＋
PLUS_COLOR = (255, 200, 61)
PLUS_LENGTH = 116
PLUS_THICKNESS = 28

# 空きマスの右へ添える +1。語ではなく数字なので距離でも潰れにくい
COUNT_TEXT = "+1"
COUNT_FONT_SIZE = 76
COUNT_GAP = 14 # 空きマスとの間隔
COUNT_COLOR = (255, 200, 61)

SLOT_RADIUS = 8

# 面の中央をうっすら明るくする。均一だと平らな板に見えて厚みが出ない
GLOW_COLOR = (74, 222, 128, 38)
GLOW_RATIO = 0.74


def load_font(names, size):
    """Windowsのシステムフォントを名前で探して読む。無ければデフォルト"""
    for name in names:
        path = os.path.join(os.environ.get("WINDIR", "C:/Windows"), "Fonts", name)
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except OSError:
                pass
    return ImageFont.load_default()


def paste_icon(image, key, x, y, size):
    """拡張子アイコンをマスの中央へ貼る"""
    path = os.path.join(ICON_DIR, f"{key}.png")
    if not os.path.exists(path):
        print(f"  警告: {path} が見つかりません")
        return

    icon = Image.open(path).convert("RGBA")

    # 素材ごとに透明な余白の量が違うため、切り落としてから大きさを揃える
    bounds = icon.getbbox()
    if bounds:
        icon = icon.crop(bounds)

    box = size * ICON_RATIO
    scale = min(box / icon.width, box / icon.height)
    icon = icon.resize((max(1, round(icon.width * scale)),
                        max(1, round(icon.height * scale))), Image.LANCZOS)
    image.paste(icon, (x + (size - icon.width) // 2, y + (size - icon.height) // 2), icon)


def draw_dashed_rect(draw, x, y, size):
    """破線の四角を描く

    実線にすると埋まっているマスと同じに見える。
    破線は「まだ何も入っていない場所」の記号として広く通じる
    """
    def dashes(start, end):
        pos = start
        while pos < end:
            yield pos, min(pos + EMPTY_DASH, end)
            pos += EMPTY_DASH + EMPTY_GAP

    for a, b in dashes(x, x + size):
        draw.line((a, y, b, y), fill=EMPTY_BORDER, width=EMPTY_WIDTH)
        draw.line((a, y + size, b, y + size), fill=EMPTY_BORDER, width=EMPTY_WIDTH)
    for a, b in dashes(y, y + size):
        draw.line((x, a, x, b), fill=EMPTY_BORDER, width=EMPTY_WIDTH)
        draw.line((x + size, a, x + size, b), fill=EMPTY_BORDER, width=EMPTY_WIDTH)


def draw_arrow_up(draw, center_x, y):
    """上向きの矢印を描く（yは上端＝矢じりの先）"""
    body_top = y + ARROW_HEAD

    draw.rectangle((center_x - ARROW_THICKNESS // 2, body_top,
                    center_x + ARROW_THICKNESS // 2, y + ARROW_HEIGHT), fill=ARROW_COLOR)
    draw.polygon([(center_x, y),
                  (center_x - ARROW_HEAD // 2, body_top),
                  (center_x + ARROW_HEAD // 2, body_top)], fill=ARROW_COLOR)


def draw_plus(draw, center_x, center_y):
    """空きマスに重ねる＋を描く"""
    half = PLUS_LENGTH // 2
    thin = PLUS_THICKNESS // 2
    draw.rectangle((center_x - half, center_y - thin, center_x + half, center_y + thin),
                   fill=PLUS_COLOR)
    draw.rectangle((center_x - thin, center_y - half, center_x + thin, center_y + half),
                   fill=PLUS_COLOR)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    image = Image.new("RGBA", (SIZE, SIZE), BOARD_COLOR + (255,))
    draw = ImageDraw.Draw(image)

    # 下に埋まったマス3つ、上に空きマス。間を上向きの矢印で繋ぐ
    filled_total = FILLED_SIZE * len(FILLED_ICONS) + FILLED_GAP * (len(FILLED_ICONS) - 1)
    left = (SIZE - filled_total) // 2
    center_x = SIZE // 2

    for i, key in enumerate(FILLED_ICONS):
        x = left + i * (FILLED_SIZE + FILLED_GAP)
        draw.rounded_rectangle((x, ROW_TOP, x + FILLED_SIZE, ROW_TOP + FILLED_SIZE),
                               radius=SLOT_RADIUS, fill=FILLED_FILL, outline=FILLED_BORDER, width=3)
        paste_icon(image, key, x, ROW_TOP, FILLED_SIZE)
        draw = ImageDraw.Draw(image)

    draw_arrow_up(draw, center_x, ARROW_TOP)

    empty_x = center_x - EMPTY_SIZE // 2
    draw_dashed_rect(draw, empty_x, EMPTY_TOP, EMPTY_SIZE)
    draw_plus(draw, center_x, EMPTY_TOP + EMPTY_SIZE // 2)

    # 空きマスの右へ +1 を添える
    font = load_font(["segoeuib.ttf", "arialbd.ttf", "consola.ttf"], COUNT_FONT_SIZE)
    box = draw.textbbox((0, 0), COUNT_TEXT, font=font)
    draw.text((empty_x + EMPTY_SIZE + COUNT_GAP,
               EMPTY_TOP + (EMPTY_SIZE - COUNT_FONT_SIZE) // 2), COUNT_TEXT,
              font=font, fill=COUNT_COLOR)

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
