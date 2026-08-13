#!/usr/bin/env python3
"""リネームブロック（拡張子の付け替え端末）のテクスチャ(PNG)を生成する。

「ここで拡張子を入れ替えられる」を、文字を読ませずに伝えることを狙う。

    F2            … 押すキー。一番上に大きく置いて最初に目へ入れる
    [ZIP] ⇄ [IMG] … 見慣れた拡張子アイコン2つを入れ替えの矢印で結ぶ
    data[.zip]    … Windowsのリネーム欄。ただし選択するのは拡張子のほう

Windowsのリネームは名前だけを選択するが、本作で書き換えるのは拡張子なので、
選択範囲を拡張子側へ置く。絵と遊びを一致させることを優先している。

下地は明るい灰白。壊して素材を得るブロックが全て暗色なので、面の色だけで
「これは壊す物ではない」と遠くからでも見分けられるようにしている。
純白にしないのは、ライティングを切っていてテクスチャの色がそのまま出るため。
白のままだと面が飛んで、乗っている絵まで読めなくなる。

アイコンは assets/images/ui/ingame/ext/ の素材を使い回す。
ブロック・インベントリ・装備スロットと同じ絵なので、
「あのアイコンを入れ替えるのだ」と既に持っている知識で読める。

出力（assets/model/stage/ 配下）:
    BlockRename.png … 512x512 テクスチャ

使い方（リポジトリのルートで実行）:
    python tools/gen_rename_texture.py
"""
import os

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ICON_DIR = os.path.join("assets", "images", "ui", "ingame", "ext")
OUT_DIR = os.path.join("assets", "model", "stage")
OUT_NAME = "BlockRename.png"

# テクスチャの一辺（他のブロックテクスチャに合わせる）
SIZE = 512

# 面の下地と縁。アイテムブロックは全て暗色なので、こちらだけ明るい面にして
# 「壊すブロックではない・操作する設備だ」を色だけで見分けられるようにする
BACKGROUND = (206, 214, 226)
BORDER_COLOR = (160, 171, 187)
BORDER_WIDTH = 6
BORDER_INSET = 6

# Windows 11のアクセント色。選択範囲の反転と矢印に使う
ACCENT = (0, 120, 212)
ACCENT_BRIGHT = (0, 103, 192)
ACCENT_GLOW = (0, 120, 212, 70)

# 文字色（白地なので暗い側を主役にする）
TEXT_BRIGHT = (250, 252, 255)
TEXT_FAINT = (60, 68, 82)

# 操作キーの案内。何をするブロックかより先に「押すキー」が目に入るよう、
# 一番上へ大きく置く
HINT_TEXT = "F2"
HINT_FONT_SIZE = 132
HINT_TOP = 10

# 入れ替えを示す2つのアイコン（左が今挿さっているもの、右が挿し替える先）
ICON_FROM = "arc"
ICON_TO = "img"
ICON_SIZE = 172
ICON_TOP = 152
ICON_SPACING = 292 # 2つのアイコンの中心どうしの間隔

# 入れ替えの矢印（双方向）
ARROW_WIDTH = 128
ARROW_THICKNESS = 14
ARROW_HEAD = 36
ARROW_GAP = 34 # 上下の矢印の間隔

# 名前を編集している欄
FIELD_WIDTH = 468
FIELD_HEIGHT = 96
FIELD_TOP = 374
FIELD_RADIUS = 8
FIELD_FILL = (232, 237, 245)

NAME_TEXT = "data"

# 矢印の先（ICON_TO）に対応する拡張子を書く。欄の中身とアイコンが食い違うと、
# 何が何に変わるのかが読み取れない
EXT_TEXT = ".png"
FIELD_FONT_SIZE = 54
CARET_WIDTH = 4


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


def text_size(draw, text, font):
    """描画したときの幅と高さを返す"""
    box = draw.textbbox((0, 0), text, font=font)
    return box[2] - box[0], box[3] - box[1]


def paste_ext_icon(image, key, center_x, top, size):
    """拡張子アイコンを余白を切り落として貼る"""
    path = os.path.join(ICON_DIR, f"{key}.png")
    if not os.path.exists(path):
        print(f"  警告: {path} が見つかりません")
        return

    icon = Image.open(path).convert("RGBA")

    # 素材ごとに透明な余白の量が違うため、切り落としてから大きさを揃える
    bounds = icon.getbbox()
    if bounds:
        icon = icon.crop(bounds)

    scale = min(size / icon.width, size / icon.height)
    icon = icon.resize((max(1, round(icon.width * scale)),
                        max(1, round(icon.height * scale))), Image.LANCZOS)
    image.paste(icon, (center_x - icon.width // 2, top), icon)


def draw_swap_arrows(draw, center_x, center_y):
    """入れ替えを示す双方向の矢印を描く

    1本の両端に矢じりを付けるより、右向きと左向きを上下に並べたほうが
    「入れ替わる」動きとして読みやすい
    """
    half = ARROW_WIDTH // 2

    for direction, offset in ((1, -ARROW_GAP // 2), (-1, ARROW_GAP // 2)):
        y = center_y + offset
        tip_x = center_x + half * direction
        tail_x = center_x - half * direction

        draw.line((tail_x, y, tip_x - ARROW_HEAD * direction, y),
                  fill=ACCENT_BRIGHT, width=ARROW_THICKNESS)
        draw.polygon([
            (tip_x, y),
            (tip_x - ARROW_HEAD * direction, y - ARROW_HEAD // 2),
            (tip_x - ARROW_HEAD * direction, y + ARROW_HEAD // 2),
        ], fill=ACCENT_BRIGHT)


def draw_name_field(draw, center_x, font):
    """名前を編集中の欄を描く

    選択するのは拡張子のほう。Windowsのリネームは名前だけを選ぶが、
    本作で書き換えるのは拡張子なので、絵と遊びを一致させる
    """
    left = center_x - FIELD_WIDTH // 2
    top = FIELD_TOP
    right = left + FIELD_WIDTH
    bottom = top + FIELD_HEIGHT

    draw.rounded_rectangle((left, top, right, bottom), radius=FIELD_RADIUS,
                           fill=FIELD_FILL, outline=ACCENT, width=3)

    name_width, _ = text_size(draw, NAME_TEXT, font)
    ext_width, _ = text_size(draw, EXT_TEXT, font)
    total = name_width + ext_width + CARET_WIDTH + 16

    text_left = center_x - total // 2
    text_top = top + (FIELD_HEIGHT - FIELD_FONT_SIZE) // 2 - 4

    draw.text((text_left, text_top), NAME_TEXT, font=font, fill=TEXT_FAINT)

    # 拡張子だけを青く反転させる。ここが書き換わる場所だと一目で分かる
    ext_left = text_left + name_width + 4
    draw.rectangle((ext_left - 6, top + 8, ext_left + ext_width + 6, bottom - 8), fill=ACCENT)
    draw.text((ext_left, text_top), EXT_TEXT, font=font, fill=TEXT_BRIGHT)

    # 末尾のカーソル。点滅は表現できないので、置いてあること自体で入力待ちを示す
    caret_x = ext_left + ext_width + 12
    draw.rectangle((caret_x, top + 12, caret_x + CARET_WIDTH, bottom - 12), fill=TEXT_BRIGHT)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    image = Image.new("RGBA", (SIZE, SIZE), BACKGROUND + (255,))
    draw = ImageDraw.Draw(image)

    draw.rectangle((BORDER_INSET, BORDER_INSET, SIZE - 1 - BORDER_INSET, SIZE - 1 - BORDER_INSET),
                   outline=BORDER_COLOR, width=BORDER_WIDTH)

    center_x = SIZE // 2

    hint_font = load_font(["segoeuib.ttf", "arialbd.ttf", "consola.ttf"], HINT_FONT_SIZE)
    hint_width, _ = text_size(draw, HINT_TEXT, hint_font)
    draw.text((center_x - hint_width // 2, HINT_TOP), HINT_TEXT, font=hint_font, fill=TEXT_FAINT)

    paste_ext_icon(image, ICON_FROM, center_x - ICON_SPACING // 2, ICON_TOP, ICON_SIZE)
    paste_ext_icon(image, ICON_TO, center_x + ICON_SPACING // 2, ICON_TOP, ICON_SIZE)

    draw = ImageDraw.Draw(image)
    draw_swap_arrows(draw, center_x, ICON_TOP + ICON_SIZE // 2)

    field_font = load_font(["consola.ttf", "cour.ttf"], FIELD_FONT_SIZE)
    draw_name_field(draw, center_x, field_font)

    # 欄の周りをうっすら光らせて、他のブロックより「操作できるもの」に見せる
    glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_draw.rounded_rectangle(
        (center_x - FIELD_WIDTH // 2 - 8, FIELD_TOP - 8,
         center_x + FIELD_WIDTH // 2 + 8, FIELD_TOP + FIELD_HEIGHT + 8),
        radius=FIELD_RADIUS + 6, outline=ACCENT_GLOW, width=10)
    glow = glow.filter(ImageFilter.GaussianBlur(radius=9))
    image = Image.alpha_composite(image, glow)

    output = os.path.join(OUT_DIR, OUT_NAME)
    image.convert("RGB").save(output)
    print(f"生成: {output}")


if __name__ == "__main__":
    main()
