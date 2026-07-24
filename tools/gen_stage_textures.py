#!/usr/bin/env python3
"""ステージ配置物のグレーボックス用テクスチャと、それを貼ったUV付き立方体(.mqo)を生成する。

設計方針（docs/design/stage_editor.md 2-4）どおり、配置物はすべて「立方体をXYZに
引き伸ばしたもの」で表現し、見た目はテクスチャで描き分ける。ゲーム側はライティングを
無効（Main.cpp の SetUseLighting(FALSE)）にしているため、テクスチャの色がそのまま出る。
＝「発光して見えるテクスチャ」を明るい色で描けば、そのままネオン風に見える。

出力（assets/model/stage/ 配下）:
    <Name>.png … 512x512 テクスチャ
    <Name>.mqo … 上記PNGを貼った 100x100x100 の両面立方体（UV付き）

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


# ---- 各配置物のテクスチャ ----

def tex_floor_folder():
    """フォルダ床：エクスプローラー詳細表示ふうの等間隔ライン＋青発光縁。"""
    img = base(VOID)
    d = ImageDraw.Draw(img)
    y = 30
    while y < SIZE:
        d.rectangle([16, y, SIZE - 16, y + 2], fill=BLUE + (90,))
        d.rectangle([26, y - 10, 42, y + 2], fill=YELLOW + (120,))  # 小さなフォルダ点
        d.rectangle([50, y - 8, 50 + 110 + (y * 7) % 150, y - 2], fill=BLUE + (70,))
        y += 44
    glow_border(img, BLUE)
    return img


def tex_path_corridor():
    """パス通路：C:\\Users\\... の文字が並ぶ通路。坂に使う。"""
    img = base((6, 10, 16))
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
    img = base((10, 16, 26))
    d = ImageDraw.Draw(img)
    # タイトルバー
    d.rectangle([0, 0, SIZE, 60], fill=(14, 22, 34, 255))
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
    """フォルダ柱：黄色い折れたフォルダ形が縦に並ぶ柱。"""
    img = base((20, 16, 5))
    d = ImageDraw.Draw(img)
    y = 40
    while y < SIZE:
        d.polygon([(120, y), (210, y), (238, y + 26), (392, y + 26),
                   (392, y + 150), (120, y + 150)], fill=YELLOW + (40,))
        y += 190
    glow_border(img, YELLOW)
    return img


def tex_block_file():
    """ファイルブロック：紙アイコン＋ .exe ラベル。"""
    img = base((11, 20, 32))
    d = ImageDraw.Draw(img)
    # 紙アイコン
    d.rectangle([SIZE / 2 - 80, 70, SIZE / 2 + 80, 300], fill=(201, 212, 227, 30))
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


TEXTURES = {
    "FloorFolder": tex_floor_folder,
    "PathCorridor": tex_path_corridor,
    "WallWindow": tex_wall_window,
    "PillarFolder": tex_pillar_folder,
    "BlockFile": tex_block_file,
    "FloorApple": tex_floor_apple,
}


# ---- UV付きの両面立方体(.mqo)を出力 ----

# 100x100x100 立方体の頂点
VERTS = [
    (-50, -50, -50), (50, -50, -50), (50, 50, -50), (-50, 50, -50),
    (-50, -50, 50), (50, -50, 50), (50, 50, 50), (-50, 50, 50),
]
# 各面の4頂点を「外から見たときの 左上→右上→右下→左下」の順で並べる。
# UVを (0,0)(1,0)(1,1)(0,1) に固定できるので、どの面でも絵が正しい向き（上が上）で貼られる。
# 両面にすると裏面のUVが鏡像になって文字が反転し、同一平面の重なりで
# Zファイティングや法線異常も起きるため、片面のみにする。
# 巻き方向の条件：連続する2辺の外積 cross(v1-v0, v2-v1) が face の外向きになること。
# 逆向きだとその面だけカリングされて消える（上下面で実際に起きた）。
FACES = [
    (3, 2, 1, 0),  # -Z（正面）
    (6, 7, 4, 5),  # +Z（背面）
    (2, 6, 5, 1),  # +X（右）
    (7, 3, 0, 4),  # -X（左）
    (7, 6, 2, 3),  # +Y（上：床の天面）
    (0, 1, 5, 4),  # -Y（下）
]
UV = [(0, 0), (1, 0), (1, 1), (0, 1)]


def mqo_text(tex_filename):
    lines = []
    lines.append("Metasequoia Document")
    lines.append("Format Text Ver 1.0")
    lines.append("")
    lines.append("Material 1 {")
    lines.append('\t"tex" shader(3) col(1.000 1.000 1.000 1.000) dif(1.000) '
                 'amb(1.000) emi(0.000) spc(0.000) power(5.00) tex("%s")' % tex_filename)
    lines.append("}")
    lines.append('Object "cube" {')
    lines.append("\tvisible 15")
    lines.append("\tlocking 0")
    lines.append("\tshading 1")
    lines.append("\tcolor 0.6 0.6 0.6")
    lines.append("\tcolor_type 0")
    lines.append("\tvertex %d {" % len(VERTS))
    for v in VERTS:
        lines.append("\t\t%.4f %.4f %.4f" % v)
    lines.append("\t}")
    lines.append("\tface %d {" % len(FACES))
    uvflat = " ".join("%.4f %.4f" % (UV[i][0], UV[i][1]) for i in range(4))
    for f in FACES:
        lines.append("\t\t4 V(%d %d %d %d) M(0) UV(%s)" % (f[0], f[1], f[2], f[3], uvflat))
    lines.append("\t}")
    lines.append("}")
    return "\n".join(lines) + "\n"


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, fn in TEXTURES.items():
        img = fn()
        png_path = os.path.join(OUT_DIR, name + ".png")
        img.convert("RGB").save(png_path)
        mqo_path = os.path.join(OUT_DIR, name + ".mqo")
        with open(mqo_path, "w", encoding="utf-8") as f:
            f.write(mqo_text(name + ".png"))
        print("generated", png_path, "/", mqo_path)


if __name__ == "__main__":
    main()
