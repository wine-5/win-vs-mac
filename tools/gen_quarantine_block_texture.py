#!/usr/bin/env python3
"""隔離フォルダ（壊すと敵が出るが、低確率で大当たりが混ざるブロック）のテクスチャ(PNG)を生成する。

面に描くのは「封じられているが、中に何か入っている」という状態。

    ╔════════╗
    ║ ＼[FILE]／ ║   ← 隔離テープが×字に封じている
    ║  ／    ＼  ║      その奥にファイルの影が薄く光る
    ╚════════╝

赤＝危険は他の画面と同じ意味で使う。隔離テープを×字に掛けて
「開けてはいけないもの」だと一目で分かるようにする。
その奥へファイルの影をシアンで薄く置く。シアンは拡張子ブロックの光と同じ色で、
「中身はプレイヤーの役に立つもの」という含みになる。

危険だけを描くと誰も壊さない。中身だけを描くと隔離である意味が無い。
封の向こうに何かが見えている状態が、危険と希望を同時に伝える唯一の形になる。

文字（「隔離」「危険」など）は入れない。
このブロックを見るのは数メートル離れた3D空間からで、語は潰れて読めない。

出力（assets/model/stage/ 配下）:
    BlockQuarantine.png … 512x512 テクスチャ

作ったあとはモデル・ひび・破片も生成すること:
    python tools/gen_stage_models.py
    python tools/gen_crack_textures.py BlockQuarantine
    python tools/gen_fracture_models.py BlockQuarantine

使い方（リポジトリのルートで実行）:
    python tools/gen_quarantine_block_texture.py
"""
import os

from PIL import Image, ImageDraw, ImageFilter

OUT_DIR = os.path.join("assets", "model", "stage")
OUT_NAME = "BlockQuarantine.png"

# テクスチャの一辺（他のブロックテクスチャに合わせる）
SIZE = 512

# 面の下地。他のブロックの暗色へ赤を混ぜ、近づく前から「これは違う」と分かるようにする
BACKGROUND = (30, 14, 16)

# 面の縁
BORDER_COLOR = (168, 44, 40)
BORDER_WIDTH = 6
BORDER_INSET = 10

# 隔離された窓（この奥に中身がある）
WINDOW_RECT = (78, 78, 434, 434)
WINDOW_RADIUS = 22
WINDOW_COLOR = (12, 16, 22)
WINDOW_EDGE_COLOR = (96, 30, 30)
WINDOW_EDGE_WIDTH = 4

# 窓の奥の中身（ファイルの影）。角を折った矩形はファイルの記号として広く通じる
FILE_WIDTH = 176
FILE_HEIGHT = 226
FILE_FOLD = 56 # 折り返す角の大きさ
FILE_COLOR = (34, 211, 238)
FILE_ALPHA = 104 # 封の向こうなので、はっきり見せない

# 中身から漏れる光。ファイルだけだと貼り絵に見え、封の奥にある感じが出ない
GLOW_COLOR = (34, 211, 238, 60)
GLOW_RATIO = 0.56

# 隔離テープ。×字に掛けて「開けてはいけない」を示す
TAPE_LENGTH = 620 # 面の対角より長くして端を切らせる
TAPE_HEIGHT = 66
TAPE_ANGLES = (26, -26)
TAPE_COLOR = (196, 34, 32)
TAPE_EDGE_COLOR = (120, 18, 18)
TAPE_EDGE_WIDTH = 4

# テープの斜め縞。無地の帯だと「板」に見えてテープだと読めない
STRIPE_COLOR = (28, 12, 12)
STRIPE_WIDTH = 20
STRIPE_PITCH = 46


def make_tape():
    """斜め縞の入った隔離テープを1本作る"""
    tape = Image.new("RGBA", (TAPE_LENGTH, TAPE_HEIGHT), TAPE_COLOR + (255,))
    draw = ImageDraw.Draw(tape)

    # 縞は垂直ではなく斜めにする。垂直だと柵に見え、テープの記号にならない。
    # 帯の高さぶん左へずらした位置から始めて、傾けたぶんの取りこぼしを無くす
    x = -TAPE_HEIGHT
    while x < TAPE_LENGTH + TAPE_HEIGHT:
        draw.polygon([(x, TAPE_HEIGHT),
                      (x + STRIPE_WIDTH, TAPE_HEIGHT),
                      (x + STRIPE_WIDTH + TAPE_HEIGHT, 0),
                      (x + TAPE_HEIGHT, 0)],
                     fill=STRIPE_COLOR + (255,))
        x += STRIPE_PITCH

    draw.rectangle((0, 0, TAPE_LENGTH - 1, TAPE_HEIGHT - 1),
                   outline=TAPE_EDGE_COLOR + (255,), width=TAPE_EDGE_WIDTH)
    return tape


def paste_centered(base, layer):
    """回転させた層を面の中央へ重ねる"""
    x = (SIZE - layer.width) // 2
    y = (SIZE - layer.height) // 2
    base.alpha_composite(layer, (x, y))


def draw_window(draw):
    """隔離された窓（中身の入れ物）を描く"""
    draw.rounded_rectangle(WINDOW_RECT, radius=WINDOW_RADIUS, fill=WINDOW_COLOR,
                           outline=WINDOW_EDGE_COLOR, width=WINDOW_EDGE_WIDTH)


def make_file_shadow():
    """窓の奥に見えるファイルの影を作る

    薄く描くので、別の層に描いてから不透明度を落として重ねる。
    直接描くと縁の重なりで濃さがまだらになる
    """
    layer = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    left = (SIZE - FILE_WIDTH) // 2
    top = (SIZE - FILE_HEIGHT) // 2
    right = left + FILE_WIDTH
    bottom = top + FILE_HEIGHT

    # 右上の角を折った矩形。折り目を描かないとただの四角に見える
    body = [(left, top),
            (right - FILE_FOLD, top),
            (right, top + FILE_FOLD),
            (right, bottom),
            (left, bottom)]
    draw.polygon(body, fill=FILE_COLOR + (FILE_ALPHA,))
    draw.polygon([(right - FILE_FOLD, top),
                  (right, top + FILE_FOLD),
                  (right - FILE_FOLD, top + FILE_FOLD)],
                 fill=FILE_COLOR + (FILE_ALPHA + 60,))
    return layer


def make_glow():
    """中身から漏れる光"""
    glow = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(glow)
    radius = int(SIZE * GLOW_RATIO * 0.5)
    center = SIZE // 2
    draw.ellipse((center - radius, center - radius, center + radius, center + radius),
                 fill=GLOW_COLOR)
    return glow.filter(ImageFilter.GaussianBlur(radius=SIZE * 0.09))


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    image = Image.new("RGBA", (SIZE, SIZE), BACKGROUND + (255,))
    draw = ImageDraw.Draw(image)
    draw_window(draw)

    image.alpha_composite(make_glow())
    image.alpha_composite(make_file_shadow())

    # テープは最後に重ねる。中身より手前にあることで「封じられている」と読める
    tape = make_tape()
    for angle in TAPE_ANGLES:
        paste_centered(image, tape.rotate(angle, expand=True, resample=Image.BICUBIC))

    draw = ImageDraw.Draw(image)
    draw.rectangle((BORDER_INSET, BORDER_INSET, SIZE - 1 - BORDER_INSET, SIZE - 1 - BORDER_INSET),
                   outline=BORDER_COLOR, width=BORDER_WIDTH)

    output = os.path.join(OUT_DIR, OUT_NAME)
    image.convert("RGB").save(output)
    print(f"生成: {output}")


if __name__ == "__main__":
    main()
