#!/usr/bin/env python3
"""破壊対象ブロックの「ひび」段階テクスチャ(PNG)を生成する。

ベースのテクスチャ1枚から、ひびが進行した段階違いのテクスチャを作る。
ゲーム側はダメージ量に応じてモデルのテクスチャを差し替えるだけでよく、
モデル(.mqo)やUVには一切手を入れない。

ひびは段階が上がるほど「増える」ように積み上げる（段階2は段階1のひびを含む）。
乱数はシード固定なので、実行するたびに同じ絵が出る。

出力（assets/model/stage/ 配下）:
    <Name>_crack1.png … 軽度のひび
    <Name>_crack2.png … 中度のひび
    <Name>_crack3.png … 重度のひび（破壊直前）

使い方（リポジトリのルートで実行）:
    python tools/gen_crack_textures.py
    python tools/gen_crack_textures.py BlockZip BlockExe
"""
import math
import os
import random
import sys

from PIL import Image, ImageDraw, ImageFilter

STAGE_DIR = os.path.join("assets", "model", "stage")

# 生成する段階数
STAGE_COUNT = 3

# 出目を固定するシード（変えるとひびの形が変わる）
SEED = 20260729

# ひびの色。芯は黒に近く、縁にシアンを乗せて「データが割れた」ように見せる
CRACK_CORE = (6, 10, 14, 235)
CRACK_EDGE = (34, 211, 238, 150)

# 破片が欠け落ちた跡（段階3のみ）
CHIP_COLOR = (8, 12, 18, 225)


def random_walk(rng, start, angle, length, steps=6, wander=0.16):
    """始点から指定方向へ、ふらつきながら伸びる折れ線を作る"""
    points = [start]
    x, y = start
    step = length / steps
    for _ in range(steps):
        angle += rng.uniform(-wander, wander)
        x += math.cos(angle) * step
        y += math.sin(angle) * step
        points.append((x, y))
    return points


def point_at(points, t):
    """折れ線上の位置（t=0.0〜1.0）を返す"""
    index = min(int(t * (len(points) - 1)), len(points) - 2)
    return points[index]


def build_cracks(rng, width, height):
    """(段階, 折れ線, 線幅) の一覧を作る"""
    cracks = []
    center = (width * 0.5 + rng.uniform(-30, 30), height * 0.46 + rng.uniform(-30, 30))

    main_count = 7
    for i in range(main_count):
        # 段階1では3本だけ、段階が上がるごとに本数が増える
        stage = 1 if i < 3 else (2 if i < 5 else 3)
        angle = (i / main_count) * math.tau + rng.uniform(-0.3, 0.3)
        length = width * rng.uniform(0.42, 0.62) * (0.55 if stage == 1 else 1.0)
        line = random_walk(rng, center, angle, length)
        cracks.append((stage, line, 9.0 if stage == 1 else 8.0))

        # 枝分かれ。親より1段階あとに現れる
        for _ in range(rng.randint(1, 3)):
            root = point_at(line, rng.uniform(0.30, 0.85))
            branch_angle = angle + rng.choice((-1.0, 1.0)) * rng.uniform(0.45, 0.95)
            branch = random_walk(rng, root, branch_angle, length * rng.uniform(0.32, 0.60), steps=4)
            cracks.append((min(STAGE_COUNT, stage + 1), branch, 4.5))

    return cracks, center


def build_chips(rng, center, count=5):
    """欠け落ちた跡（不定形の多角形）を作る"""
    chips = []
    for _ in range(count):
        distance = rng.uniform(20, 110)
        angle = rng.uniform(0, math.tau)
        cx = center[0] + math.cos(angle) * distance
        cy = center[1] + math.sin(angle) * distance
        radius = rng.uniform(11, 24)
        polygon = []
        for i in range(rng.randint(5, 7)):
            a = (i / 6) * math.tau
            r = radius * rng.uniform(0.6, 1.35)
            polygon.append((cx + math.cos(a) * r, cy + math.sin(a) * r))
        chips.append(polygon)
    return chips


def render_stage(base, cracks, chips, stage):
    """指定段階までのひびをベース画像へ焼き込む"""
    width, height = base.size

    # 縁のシアンをぼかして光らせるため、芯と縁を別レイヤーに描く
    edge_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    core_layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    edge_draw = ImageDraw.Draw(edge_layer)
    core_draw = ImageDraw.Draw(core_layer)

    for crack_stage, line, line_width in cracks:
        if crack_stage > stage:
            continue
        edge_draw.line(line, fill=CRACK_EDGE, width=int(line_width) + 3, joint="curve")
        core_draw.line(line, fill=CRACK_CORE, width=max(1, int(line_width)), joint="curve")

    if stage >= STAGE_COUNT:
        for polygon in chips:
            edge_draw.polygon(polygon, outline=CRACK_EDGE)
            core_draw.polygon(polygon, fill=CHIP_COLOR)

    edge_layer = edge_layer.filter(ImageFilter.GaussianBlur(radius=2.2))

    result = base.convert("RGBA")
    result = Image.alpha_composite(result, edge_layer)
    result = Image.alpha_composite(result, core_layer)
    return result.convert("RGB")


def generate(name):
    source = os.path.join(STAGE_DIR, f"{name}.png")
    if not os.path.exists(source):
        print(f"  スキップ: {source} が見つかりません")
        return

    base = Image.open(source).convert("RGB")
    rng = random.Random(SEED)
    cracks, center = build_cracks(rng, *base.size)
    chips = build_chips(rng, center)

    for stage in range(1, STAGE_COUNT + 1):
        output = os.path.join(STAGE_DIR, f"{name}_crack{stage}.png")
        render_stage(base, cracks, chips, stage).save(output)
        print(f"  生成: {output}")


def main():
    targets = sys.argv[1:] or ["BlockZip"]
    for name in targets:
        print(f"{name}:")
        generate(name)


if __name__ == "__main__":
    main()
