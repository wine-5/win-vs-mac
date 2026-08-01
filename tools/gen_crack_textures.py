#!/usr/bin/env python3
"""破壊対象ブロックの「ひび」段階テクスチャ(PNG)を生成する。

ベースのテクスチャ1枚から、ひびが進行した段階違いのテクスチャを作る。
ゲーム側はダメージ量に応じてモデルのテクスチャを差し替えるだけでよく、
モデル(.mqo)やUVには一切手を入れない。

ひびの形は実際のガラス破壊の構造をなぞる:
    - 衝撃点から外へ伸びる「放射ひび」
    - その放射ひびを横切って渡る「同心ひび」（放射だけだと放射状の模様に見えてしまう）
    - 先端へ向かって細くなる先細り
    - 縁のハイライト（面取り）で溝の深さを出す

段階が上がるほどひびは「増える」ように積み上げる（段階2は段階1のひびを含む）。
乱数のシードはブロック名から作るので、実行するたびに同じ絵が出るが、
ブロックごとには違う割れ方になる。

出力（assets/model/stage/ 配下）:
    <Name>_crack1.png … 軽度のひび
    <Name>_crack2.png … 中度のひび
    <Name>_crack3.png … 重度のひび（破壊直前）

使い方（リポジトリのルートで実行）:
    python tools/gen_crack_textures.py
    python tools/gen_crack_textures.py BlockZip
"""
import math
import os
import random
import sys
import zlib

from PIL import Image, ImageDraw, ImageFilter

STAGE_DIR = os.path.join("assets", "model", "stage")

# 生成する段階数
STAGE_COUNT = 3

# シードの土台。変えると全ブロックの割れ方が一斉に変わる
SEED_BASE = 20260730

# ひびの芯。ほぼ黒で「向こう側が見えない溝」にする
CRACK_CORE = (4, 6, 9, 250)

# 溝の縁で光を拾う面。これが無いと描いた線にしか見えない
CRACK_HIGHLIGHT = (206, 226, 240, 105)

# 溝が落とす影。芯の下に広めに敷いて深さを出す
CRACK_SHADOW = (0, 0, 0, 120)

# 衝撃点の砕けた領域。単色で塗るとシールを貼ったように見えるため、
# 短いひびの密集と、その下に敷く淡い影で表現する
IMPACT_HAZE = (0, 0, 0, 90)
IMPACT_MICRO_COUNT = 18
IMPACT_MICRO_RADIUS = 22.0

# 欠け落ちた跡（段階3のみ）
CHIP_COLOR = (3, 5, 8, 235)

# 放射ひびの本数
RADIAL_COUNT = 11

# 放射ひびの根元の太さ／先端の太さ（ピクセル）
RADIAL_WIDTH_ROOT = 7.0
RADIAL_WIDTH_TIP = 1.0

# 同心ひびの太さ
RING_WIDTH = 2.4

# 縁のハイライトをずらす量（光源が左上にある想定）
HIGHLIGHT_OFFSET = (-1.6, -1.6)

# 既定の生成対象。破壊できるブロック（blockTableの抽選対象）をすべて挙げる
DEFAULT_TARGETS = [
    "BlockZip",
    "BlockExtArchive",
    "BlockExtAudio",
    "BlockExtDocument",
    "BlockExtExecutable",
    "BlockExtImage",
    "BlockExtShortcut",
    "BlockExtSourceCode",
    "BlockExtVideo",
    "BlockExtUnknown",
]


def make_rng(name):
    """ブロック名から乱数エンジンを作る

    ブロックごとに違う割れ方にしつつ、同じ名前なら毎回同じ絵にするため、
    名前のCRC32をシードに混ぜる。Pythonのhash()は実行のたびに変わるので使わない。
    """
    return random.Random(SEED_BASE ^ zlib.crc32(name.encode("utf-8")))


def radial_line(rng, center, angle, length, segments=8):
    """衝撃点から外へ伸びる放射ひびの折れ線を作る

    ガラスのひびはほぼ直進し、たまにカクッと向きを変える。
    毎ステップ大きく揺らすと植物の根のように見えてしまうため、
    ふらつきは小さく保ち、稀に大きく折れるようにしている。
    """
    points = [center]
    x, y = center
    step = length / segments
    for _ in range(segments):
        angle += rng.uniform(-0.07, 0.07)
        if rng.random() < 0.18:
            angle += rng.choice((-1.0, 1.0)) * rng.uniform(0.14, 0.30)
        x += math.cos(angle) * step
        y += math.sin(angle) * step
        points.append((x, y))
    return points


def point_on(points, t):
    """折れ線上の位置（t=0.0〜1.0）を線形補間で返す"""
    if t <= 0.0:
        return points[0]
    if t >= 1.0:
        return points[-1]
    span = t * (len(points) - 1)
    index = int(span)
    frac = span - index
    x0, y0 = points[index]
    x1, y1 = points[index + 1]
    return (x0 + (x1 - x0) * frac, y0 + (y1 - y0) * frac)


def ring_line(rng, center, start, end):
    """2本の放射ひびの間を渡る同心ひびを作る

    衝撃点を中心とする円弧に近い形にしたいので、両端の中点を
    中心から見て外側へ少し押し出し、そこを通る折れ線にする。
    """
    points = [start]
    steps = 4
    for i in range(1, steps):
        t = i / steps
        x = start[0] + (end[0] - start[0]) * t
        y = start[1] + (end[1] - start[1]) * t

        # 中心から離れる向きへ膨らませて円弧に寄せる
        dx, dy = x - center[0], y - center[1]
        distance = math.hypot(dx, dy) or 1.0
        bulge = math.sin(t * math.pi) * rng.uniform(0.06, 0.13)
        x += dx / distance * distance * bulge
        y += dy / distance * distance * bulge

        x += rng.uniform(-3.0, 3.0)
        y += rng.uniform(-3.0, 3.0)
        points.append((x, y))
    points.append(end)
    return points


def build_fracture(rng, width, height):
    """(段階, 折れ線, 根元の太さ, 先端の太さ) の一覧と衝撃点を返す"""
    center = (width * 0.5 + rng.uniform(-22, 22), height * 0.48 + rng.uniform(-22, 22))
    max_length = width * 0.58

    radials = []
    cracks = []
    for i in range(RADIAL_COUNT):
        # 均等割りだと放射状の模様に見えるので、角度をばらす
        angle = (i / RADIAL_COUNT) * math.tau + rng.uniform(-0.22, 0.22)

        # 段階1は少数の長いひびだけ。段階が上がるごとに間を埋めていく
        if i % 4 == 0:
            stage, scale = 1, rng.uniform(0.62, 0.82)
        elif i % 2 == 0:
            stage, scale = 2, rng.uniform(0.72, 1.0)
        else:
            stage, scale = 3, rng.uniform(0.55, 0.95)

        line = radial_line(rng, center, angle, max_length * scale)
        radials.append((stage, line))
        cracks.append((stage, line, RADIAL_WIDTH_ROOT * (0.8 + 0.2 * scale), RADIAL_WIDTH_TIP))

    # 同心ひび。隣り合う放射ひびの間を渡す
    for i in range(RADIAL_COUNT):
        stage_a, line_a = radials[i]
        stage_b, line_b = radials[(i + 1) % RADIAL_COUNT]

        # 両方の放射ひびが出そろってからでないと渡せない
        base_stage = max(stage_a, stage_b)
        for ring_stage in range(max(2, base_stage), STAGE_COUNT + 1):
            if rng.random() > 0.72:
                continue
            t = rng.uniform(0.25, 0.85)
            start = point_on(line_a, t)
            end = point_on(line_b, t + rng.uniform(-0.10, 0.10))
            cracks.append((ring_stage, ring_line(rng, center, start, end),
                           RING_WIDTH, RING_WIDTH * 0.7))

    return cracks, center


def build_chips(rng, center, count=3):
    """欠け落ちた跡（不定形の多角形）を作る"""
    chips = []
    for _ in range(count):
        distance = rng.uniform(24, 90)
        angle = rng.uniform(0, math.tau)
        cx = center[0] + math.cos(angle) * distance
        cy = center[1] + math.sin(angle) * distance
        radius = rng.uniform(5, 11)
        polygon = []
        corners = rng.randint(5, 7)
        for i in range(corners):
            a = (i / corners) * math.tau
            r = radius * rng.uniform(0.55, 1.4)
            polygon.append((cx + math.cos(a) * r, cy + math.sin(a) * r))
        chips.append(polygon)
    return chips


def build_impact(rng, center):
    """衝撃点の砕けた領域を、短いひびの密集として作る

    実際の粉砕は「細かいひびが密集して光を乱反射している」状態なので、
    塗りつぶしではなく短いひびを放射状に敷き詰めて表現する。
    """
    micro = []
    for i in range(IMPACT_MICRO_COUNT):
        angle = (i / IMPACT_MICRO_COUNT) * math.tau + rng.uniform(-0.2, 0.2)
        inner = rng.uniform(1.5, 5.0)
        outer = inner + rng.uniform(6.0, IMPACT_MICRO_RADIUS)
        micro.append([
            (center[0] + math.cos(angle) * inner, center[1] + math.sin(angle) * inner),
            (center[0] + math.cos(angle + rng.uniform(-0.15, 0.15)) * outer,
             center[1] + math.sin(angle + rng.uniform(-0.15, 0.15)) * outer),
        ])
    return micro


def draw_tapered(draw, points, width_root, width_tip, color, offset=(0.0, 0.0)):
    """先細りする折れ線を描く

    ImageDrawのlineは1本につき1つの太さしか持てないため、区間ごとに太さを変えて
    描き、継ぎ目を円で埋めて滑らかにつなぐ。
    """
    segments = len(points) - 1
    if segments <= 0:
        return

    shifted = [(x + offset[0], y + offset[1]) for x, y in points]
    for i in range(segments):
        t = i / segments
        line_width = max(1, round(width_root + (width_tip - width_root) * t))
        draw.line([shifted[i], shifted[i + 1]], fill=color, width=line_width)

        # 太い区間は継ぎ目が角張るので円で埋める
        if line_width > 2:
            radius = line_width / 2.0
            x, y = shifted[i + 1]
            draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=color)


def render_stage(base, cracks, chips, impact, stage):
    """指定段階までのひびをベース画像へ焼き込む"""
    visible = [c for c in cracks if c[0] <= stage]

    shadow_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    highlight_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    core_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    shadow_draw = ImageDraw.Draw(shadow_layer)
    highlight_draw = ImageDraw.Draw(highlight_layer)
    core_draw = ImageDraw.Draw(core_layer)

    for _, line, width_root, width_tip in visible:
        draw_tapered(shadow_draw, line, width_root + 4.0, width_tip + 3.0, CRACK_SHADOW)
        draw_tapered(highlight_draw, line, width_root, width_tip,
                     CRACK_HIGHLIGHT, HIGHLIGHT_OFFSET)
        draw_tapered(core_draw, line, width_root, width_tip, CRACK_CORE)

    if stage >= STAGE_COUNT:
        for polygon in chips:
            core_draw.polygon(polygon, fill=CHIP_COLOR)
            highlight_draw.polygon(
                [(x + HIGHLIGHT_OFFSET[0], y + HIGHLIGHT_OFFSET[1]) for x, y in polygon],
                outline=CRACK_HIGHLIGHT)

    # 衝撃点。段階が進むほど密集したひびが増えて濁って見える
    visible_micro = int(len(impact) * (0.4 + 0.3 * stage))
    for line in impact[:visible_micro]:
        draw_tapered(shadow_draw, line, 4.0, 3.0, IMPACT_HAZE)
        draw_tapered(highlight_draw, line, 2.0, 1.0, CRACK_HIGHLIGHT, HIGHLIGHT_OFFSET)
        draw_tapered(core_draw, line, 2.0, 1.0, CRACK_CORE)

    shadow_layer = shadow_layer.filter(ImageFilter.GaussianBlur(radius=2.6))
    highlight_layer = highlight_layer.filter(ImageFilter.GaussianBlur(radius=0.6))

    result = base.convert("RGBA")
    result = Image.alpha_composite(result, shadow_layer)
    result = Image.alpha_composite(result, highlight_layer)
    result = Image.alpha_composite(result, core_layer)
    return result.convert("RGB")


def generate(name):
    source = os.path.join(STAGE_DIR, f"{name}.png")
    if not os.path.exists(source):
        print(f"  スキップ: {source} が見つかりません")
        return

    base = Image.open(source).convert("RGB")
    rng = make_rng(name)
    cracks, center = build_fracture(rng, *base.size)
    chips = build_chips(rng, center)
    impact = build_impact(rng, center)

    for stage in range(1, STAGE_COUNT + 1):
        output = os.path.join(STAGE_DIR, f"{name}_crack{stage}.png")
        render_stage(base, cracks, chips, impact, stage).save(output)
        print(f"  生成: {output}")


def main():
    targets = sys.argv[1:] or DEFAULT_TARGETS
    for name in targets:
        print(f"{name}:")
        generate(name)


if __name__ == "__main__":
    main()
