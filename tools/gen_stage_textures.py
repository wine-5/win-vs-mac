#!/usr/bin/env python3
"""ステージ配置物のグレーボックス用テクスチャ(PNG)を生成する。

配置物はすべて「立方体をXYZに引き伸ばしたもの」で表現し、見た目はテクスチャで
描き分ける。ゲーム側はライティングを無効（Main.cpp の SetUseLighting(FALSE)）に
しているため、テクスチャの色がそのまま出る。

立方体(.mqo)の生成は gen_stage_models.py に分離した。テクスチャを外部（写実的な
画像）へ移行しても、幾何・UVはそちらで一元管理する。このツールは当面の仮テクスチャや
プレースホルダを作る用途で残す。

出力（assets/model/stage/ 配下）:
    <Name>.png … 512x512 テクスチャ

使い方（リポジトリのルートで実行）:
    python tools/gen_stage_textures.py
"""
import os
from PIL import Image, ImageDraw, ImageFont, ImageFilter

OUT_DIR = os.path.join("assets", "model", "stage")
SIZE = 512

# ---- 色（docs/design/stage_editor.md 2-3） ----
VOID = (7, 12, 19)
VOID2 = (10, 14, 20)
BLUE = (0, 164, 239)
BLUE_DIM = (0, 95, 138)
YELLOW = (255, 200, 61)
RED = (232, 17, 35)
APPLE = (232, 232, 237)
APPLE2 = (245, 245, 247)
TEXT = (201, 212, 227)


def load_font(names, size):
    """Windowsのシステムフォントを名前で探して読む。無ければデフォルト。"""
    for name in names:
        path = os.path.join(os.environ.get("WINDIR", "C:/Windows"), "Fonts", name)
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except OSError:
                pass
    return ImageFont.load_default()


MONO = lambda s: load_font(["consola.ttf", "cour.ttf"], s)
SANS = lambda s: load_font(["segoeui.ttf", "meiryo.ttc", "arial.ttf"], s)


def glow_border(img, color, width=12, blur=18):
    """発光する縁を合成する（PILにブルームは無いのでガウスぼかしで近似）。"""
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    w, h = img.size
    d.rectangle([width // 2, width // 2, w - width // 2, h - width // 2],
                outline=color + (255,), width=width)
    blurred = layer.filter(ImageFilter.GaussianBlur(blur))
    img.alpha_composite(blurred)
    img.alpha_composite(layer)


def base(color=VOID):
    return Image.new("RGBA", (SIZE, SIZE), color + (255,))


def surface_base(bg, accent, grid_step=64):
    """面を「立体の一部」として認識させるための下地を作る。

    背景が真っ黒な虚無のため、ベースまで暗いと面が背景に溶けて箱に見えなくなる。
    ベースを虚無より明確に明るくし、控えめなグリッドで面の広がりを、
    発光する縁で箱の稜線を示す。この上に種類ごとのモチーフを載せる。
    """
    img = base(bg)
    d = ImageDraw.Draw(img)
    for x in range(grid_step, SIZE, grid_step):
        d.line([x, 0, x, SIZE], fill=accent + (36,), width=1)
    for y in range(grid_step, SIZE, grid_step):
        d.line([0, y, SIZE, y], fill=accent + (36,), width=1)
    return img


# ---- 各配置物のテクスチャ ----

def tex_floor_folder():
    """フォルダ床：エクスプローラー詳細表示ふうの等間隔ライン＋青発光縁。"""
    img = surface_base((18, 30, 46), BLUE)
    d = ImageDraw.Draw(img)
    y = 44
    while y < SIZE - 20:
        d.rectangle([28, y, SIZE - 28, y + 3], fill=BLUE + (150,))
        d.rectangle([40, y - 14, 58, y - 2], fill=YELLOW + (190,))  # 小さなフォルダ点
        d.rectangle([70, y - 12, 70 + 120 + (y * 7) % 160, y - 4], fill=BLUE + (110,))
        y += 64
    glow_border(img, BLUE)
    return img


def tex_path_corridor():
    """パス通路：C:\\Users\\... の文字が並ぶ通路。坂に使う。"""
    img = surface_base((16, 28, 44), BLUE)
    d = ImageDraw.Draw(img)
    f = MONO(30)
    lines = ["C:\\Users\\Player\\", "Desktop > ..", "C:\\Program Files\\", ">  >  >"]
    for i in range(8):
        col = BLUE + (220,) if i % 2 else BLUE + (110,)
        d.text((SIZE / 2, 40 + i * 62), lines[i % len(lines)], font=f, fill=col, anchor="mm")
    glow_border(img, BLUE, width=10, blur=14)
    return img


def tex_wall_window():
    """ウィンドウ壁：巨大なエクスプローラーの枠。タイトルバー＋ − □ ✕。"""
    img = surface_base((20, 32, 50), BLUE)
    d = ImageDraw.Draw(img)
    # タイトルバー
    d.rectangle([0, 0, SIZE, 60], fill=(30, 46, 68, 255))
    d.text((16, 30), "C:\\Program Files", font=MONO(26), fill=TEXT + (230,), anchor="lm")
    # - □ ×
    d.text((SIZE - 150, 28), "\u2014", font=SANS(30), fill=(143, 160, 181, 255), anchor="mm")
    d.rectangle([SIZE - 108, 16, SIZE - 84, 40], outline=(143, 160, 181, 255), width=3)
    d.text((SIZE - 45, 28), "\u2715", font=SANS(30), fill=RED + (255,), anchor="mm")
    # 本体のうっすらグリッド
    for x in range(0, SIZE, 64):
        d.line([x, 60, x, SIZE], fill=BLUE + (18,), width=1)
    for y in range(60, SIZE, 64):
        d.line([0, y, SIZE, y], fill=BLUE + (18,), width=1)
    glow_border(img, BLUE)
    return img


def tex_pillar_folder():
    """フォルダ柱：黄色いフォルダアイコンが並ぶ柱。"""
    img = surface_base((46, 38, 16), YELLOW)
    d = ImageDraw.Draw(img)
    # フォルダアイコンを小さめに、縦に間隔をあけて並べる
    for y in (96, 288):
        left, right = 168, 344
        d.polygon([(left, y), (left + 58, y), (left + 78, y + 20), (right, y + 20),
                   (right, y + 118), (left, y + 118)], fill=YELLOW + (235,))
    glow_border(img, YELLOW)
    return img


def tex_block_file():
    """ファイルブロック：紙アイコン＋ .exe ラベル。"""
    img = surface_base((30, 44, 62), BLUE)
    d = ImageDraw.Draw(img)
    # 紙アイコン
    d.rectangle([SIZE / 2 - 80, 70, SIZE / 2 + 80, 300], fill=(201, 212, 227, 120))
    d.polygon([(SIZE / 2 + 40, 70), (SIZE / 2 + 80, 110), (SIZE / 2 + 40, 110)],
              fill=(201, 212, 227, 70))
    d.text((SIZE / 2, 380), ".exe", font=MONO(72), fill=BLUE + (255,), anchor="mm")
    glow_border(img, BLUE)
    return img


def tex_floor_apple():
    """アリーナ床：白銀の放射グラデ＋赤い危険サークル。"""
    img = base(APPLE)
    d = ImageDraw.Draw(img)
    cx = cy = SIZE / 2
    # 放射グラデ（中心が明るく外周が暗い銀）を同心円で近似
    for r in range(int(SIZE / 2), 0, -2):
        t = r / (SIZE / 2)
        c = (int(250 - 40 * t), int(250 - 38 * t), int(252 - 30 * t))
        d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=c + (255,))
    # 赤い危険リング
    d.ellipse([cx - 150, cy - 150, cx + 150, cy + 150], outline=RED + (140,), width=6)
    d.ellipse([cx - 170, cy - 170, cx + 170, cy + 170], outline=RED + (60,), width=3)
    return img


# データ壁に流すログ・コード行。ターゲットがプログラマーなので、
# それらしい専門用語を並べて「システムの内側を覗いている」感じを出す
DATA_LINES = [
    "[ok] kernel32.dll  mapped 0x7ffb2c40",
    "  mov  rax, qword ptr [rsp+28h]",
    "[warn] handle leak  pid=4812 count=137",
    "  thread 0x1a4  state=WAITING",
    "[ok] ntfs  journal flush  12.4MB/s",
    "  if (hr != S_OK) return hr;",
    "[intruder] apple.process  signature mismatch",
    "  call  QueryPerformanceCounter",
    "[ok] page fault  soft=1284 hard=3",
    "  lock cmpxchg [rbx], rcx",
    "[warn] gdi objects  9821 / 10000",
    "  0x00 0x4d 0x5a 0x90 0x00 0x03",
    "[ok] scheduler  quantum=15ms",
    "  while (!queue.empty()) { pop(); }",
    "[intruder] safari.exe  spawned child",
    "  ret",
]


def tex_wall_data():
    """データ壁：ログとコードが並ぶ面。

    描画側でUVを縦にずらして流すため、**上下が繋がる**ように作る。
    行を等間隔に並べ、上下端に半端な行を残さないことでシームレスになる。
    上下に縁を入れると繋ぎ目が出るので、光らせるのは左右だけにする。
    """
    img = surface_base((14, 26, 40), BLUE, grid_step=64)
    d = ImageDraw.Draw(img)
    f = MONO(18)

    # 行間は SIZE を割り切れる値にする（繰り返しても行が途切れない）
    line_height = SIZE // len(DATA_LINES)
    for i, text in enumerate(DATA_LINES):
        y = i * line_height + line_height // 2
        # 種別で色を変え、ログとしての情報の粒度を出す
        if text.startswith("[intruder]"):
            color = RED + (235,)
        elif text.startswith("[warn]"):
            color = YELLOW + (215,)
        elif text.startswith("[ok]"):
            color = BLUE + (215,)
        else:
            color = (120, 200, 245, 160)  # コード行は控えめに
        d.text((16, y), text, font=f, fill=color, anchor="lm")

    d.rectangle([0, 0, 6, SIZE], fill=BLUE + (220,))
    d.rectangle([SIZE - 7, 0, SIZE - 1, SIZE], fill=BLUE + (220,))
    return img


TEXTURES = {
    "WallData": tex_wall_data,
    "FloorFolder": tex_floor_folder,
    "PathCorridor": tex_path_corridor,
    "WallWindow": tex_wall_window,
    "PillarFolder": tex_pillar_folder,
    "BlockFile": tex_block_file,
    "FloorApple": tex_floor_apple,
}


def main():
    # このツールはテクスチャ(PNG)のみを生成する。立方体(.mqo)の生成は
    # gen_stage_models.py に分離した（テクスチャを外部生成へ移行するため）。
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, fn in TEXTURES.items():
        img = fn()
        png_path = os.path.join(OUT_DIR, name + ".png")
        img.convert("RGB").save(png_path)
        print("generated", png_path)


if __name__ == "__main__":
    main()
