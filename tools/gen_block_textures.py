#!/usr/bin/env python3
"""拡張子ブロック（ファイル単体のブロック）のテクスチャ(PNG)を生成する。

InGameのHUDで使っている拡張子アイコン（assets/images/ui/ingame/ext/*.png）を
そのままブロックの面へ拡大して貼る。

こうする理由は「壊したブロックの絵」と「スロットに入る欠片の絵」を一致させるため。
プレイヤーは文字を読まずに「この絵のブロックを壊すとこの絵が手に入る」と学習できる。
アイコン側が既に色（ラベルチップ）と記号の両方で区別されているので、
遠くて文字が潰れても色と記号で判別できる。

出力（assets/model/stage/ 配下）:
    BlockExt<Name>.png … 512x512 テクスチャ

使い方（リポジトリのルートで実行）:
    python tools/gen_block_textures.py
"""
import os

from PIL import Image, ImageDraw, ImageFilter

ICON_DIR = os.path.join("assets", "images", "ui", "ingame", "ext")
OUT_DIR = os.path.join("assets", "model", "stage")

# テクスチャの一辺（既存のブロックテクスチャに合わせる）
SIZE = 512

# 面の下地。既存の BlockZip / BlockExe と同系の暗色
BACKGROUND = (17, 20, 26)

# 面の縁。立方体の輪郭を出して「面」として認識させる
BORDER_COLOR = (44, 58, 74)
BORDER_WIDTH = 6
BORDER_INSET = 10

# アイコンが面に占める比率。素材の透明余白を切り落としてから当てるので、
# この値がそのまま「面に対する絵の大きさ」になる
ICON_RATIO = 0.84

# アイコンの背後に敷く光。暗い面に暗いアイコンが沈むのを防ぐ
GLOW_COLOR = (34, 211, 238, 46)
GLOW_RATIO = 0.78

# 生成対象。emp（空きスロット）はブロックにならないので除く
TARGETS = {
    "arc": "Archive",
    "aud": "Audio",
    "doc": "Document",
    "exe": "Executable",
    "img": "Image",
    "lnk": "Shortcut",
    "src": "SourceCode",
    "vid": "Video",
    "etc": "Unknown",
}


def make_background():
    """面の下地（縁つき）を作る"""
    image = Image.new("RGBA", (SIZE, SIZE), BACKGROUND + (255,))
    draw = ImageDraw.Draw(image)
    draw.rectangle(
        (BORDER_INSET, BORDER_INSET, SIZE - 1 - BORDER_INSET, SIZE - 1 - BORDER_INSET),
        outline=BORDER_COLOR + (255,),
        width=BORDER_WIDTH,
    )
    return image


def make_glow():
    """アイコンの背後に敷く柔らかい光"""
    glow = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(glow)
    radius = int(SIZE * GLOW_RATIO * 0.5)
    center = SIZE // 2
    draw.ellipse(
        (center - radius, center - radius, center + radius, center + radius),
        fill=GLOW_COLOR,
    )
    return glow.filter(ImageFilter.GaussianBlur(radius=SIZE * 0.09))


def generate(key, name, background, glow):
    source = os.path.join(ICON_DIR, f"{key}.png")
    if not os.path.exists(source):
        print(f"  スキップ: {source} が見つかりません")
        return

    icon = Image.open(source).convert("RGBA")

    # 素材ごとに透明な余白の量が違うため、先に切り落として大きさを揃える。
    # これをしないと同じ倍率でも絵の大きさがアイコンごとにばらつく
    bounds = icon.getbbox()
    if bounds:
        icon = icon.crop(bounds)

    # 縦横比を保ったまま、面に対して ICON_RATIO の大きさへ収める
    box = SIZE * ICON_RATIO
    scale = min(box / icon.width, box / icon.height)
    icon = icon.resize((max(1, round(icon.width * scale)),
                        max(1, round(icon.height * scale))), Image.LANCZOS)

    result = background.copy()
    result = Image.alpha_composite(result, glow)

    result.paste(icon, ((SIZE - icon.width) // 2, (SIZE - icon.height) // 2), icon)

    output = os.path.join(OUT_DIR, f"BlockExt{name}.png")
    result.convert("RGB").save(output)
    print(f"  生成: {output}")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    background = make_background()
    glow = make_glow()
    print("拡張子ブロック:")
    for key, name in TARGETS.items():
        generate(key, name, background, glow)


if __name__ == "__main__":
    main()
