#!/usr/bin/env python3
"""コンセプト（prototype/stage-edito-prototyper.html）の5区画ルートをステージJSONへ変換する。

HTMLのプロトタイプと同じ計算をここで再現することで、座標を手で写して
ズレるのを防ぐ。レイアウトを変えたいときはHTMLではなくこのスクリプトを直す。

ルート:
  ① Desktop → 坂A → ② Downloads → 90°通路 → つづら折り坂B1/B2
  → ③ System32（クランク通路・Temp寄り道）→ 坂C → ④ Program Files
  → UACゲート → 坂D → ⑤ Apple アリーナ

使い方（リポジトリのルートで実行）:
    python tools/gen_stage_from_concept.py [出力先]
"""
import json
import math
import sys

SLOPE_DEG = 13.0
FLOOR_THICKNESS = 24.0

props = []
spawns = []
lights = []


def add_prop(kind, pos, size, rot=(0.0, 0.0, 0.0)):
    props.append({
        "type": kind,
        "position": [round(v, 2) for v in pos],
        "rotation": [round(v, 2) for v in rot],
        "size": [round(v, 2) for v in size],
    })


def add_floor(cx, top_y, cz, sx, sz, kind="floor_folder"):
    """天面の高さを top_y として床を置く（プロトタイプの addFloor と同じ基準）"""
    add_prop(kind, (cx, top_y - FLOOR_THICKNESS / 2, cz), (sx, FLOOR_THICKNESS, sz))


def add_slope(start_x, top_y, start_z, length, width, axis, kind="path_corridor"):
    """始点（床の縁・天面高さ）から axis 方向へ下る坂。終点を返す"""
    rad = math.radians(SLOPE_DEG)
    drop = length * math.sin(rad)
    span = length * math.cos(rad)

    if axis == "z+":
        add_prop(kind, (start_x, top_y - FLOOR_THICKNESS / 2 - drop / 2, start_z + span / 2),
                 (width, FLOOR_THICKNESS, length), (SLOPE_DEG, 0.0, 0.0))
        return start_x, top_y - drop, start_z + span

    # 'x-' : -x 方向へ下る。Y90°回して坂の向きを合わせる
    add_prop(kind, (start_x - span / 2, top_y - FLOOR_THICKNESS / 2 - drop / 2, start_z),
             (width, FLOOR_THICKNESS, length), (SLOPE_DEG, 90.0, 0.0))
    return start_x - span, top_y - drop, start_z


def add_corridor(cx, top_y, cz, length, axis, width=1000.0, wall_h=620.0):
    """System32のクランク通路：床＋両側のデータ壁（情報が流れる壁）"""
    along_z = axis == "z"
    add_floor(cx, top_y, cz, width if along_z else length, length if along_z else width)
    for side in (-1, 1):
        if along_z:
            add_prop("wall_data", (cx + side * (width / 2 + 15), top_y + wall_h / 2, cz),
                     (length, wall_h, 30), (0.0, 90.0, 0.0))
        else:
            add_prop("wall_data", (cx, top_y + wall_h / 2, cz + side * (width / 2 + 15)),
                     (length, wall_h, 30))


def add_window_wall(cx, top_y, cz, w, h, ry=0.0):
    add_prop("wall_window", (cx, top_y + h / 2, cz), (w, h, 40), (0.0, ry, 0.0))


def add_pillar(x, floor_y, z):
    add_prop("pillar_folder", (x, floor_y + 310, z), (240, 620, 240))


def add_block(x, floor_y, z, ry=0.0, size=220.0, lift=0.0):
    add_prop("block_file", (x, floor_y + size / 2 + lift, z), (size, size, size), (0.0, ry, 0.0))


def add_enemy(kind, x, floor_y, z, rot_y=0.0):
    spawns.append({"type": kind, "position": [round(x, 2), round(floor_y, 2), round(z, 2)],
                   "rotationY": rot_y})


def add_light(x, y, z, rng, color):
    lights.append({"position": [round(x, 2), round(y, 2), round(z, 2)],
                   "range": rng, "color": list(color)})


BLUE = (0, 164, 239)
YELLOW = (255, 200, 61)
WHITE = (245, 245, 247)
RED = (232, 17, 35)

# ---- ① Desktop ----
Y1 = 0.0
add_floor(0, Y1, 0, 2600, 2600)
add_window_wall(0, Y1, -1350, 3000, 1500)
add_block(-900, Y1, -700, 23)
add_block(950, Y1, -800, -17)
add_enemy("safari", -500, Y1, 300)
add_enemy("safari", 550, Y1, 150)
add_light(0, Y1 + 900, 0, 6000, BLUE)

# ---- 坂A ----
sA_x, sA_y, sA_z = add_slope(0, Y1, 1300, 2600, 1000, "z+")

# ---- ② Downloads ----
Y2 = sA_y
R2Z = sA_z + 1400
add_floor(0, Y2, R2Z, 2800, 2800)
add_window_wall(-1420, Y2, R2Z, 2800, 1400, 90)
for bx, bz, br, lift in ((-700, -500, 29, 0), (-450, -350, -46, 0), (-580, -430, 6, 220),
                         (800, 600, 63, 0), (300, 900, -29, 0)):
    add_block(bx, Y2, R2Z + bz, br, lift=lift)
add_enemy("safari", -600, Y2, R2Z + 400)
add_enemy("safari", 700, Y2, R2Z - 300)
add_enemy("xcode", 100, Y2, R2Z + 900)
add_light(0, Y2 + 900, R2Z, 6000, BLUE)

# ---- 90°ターン ----
T1X, T1LEN = 1400.0, 1900.0
add_floor(T1X + T1LEN / 2, Y2, R2Z + 500, T1LEN, 1000)
J1X = T1X + T1LEN + 500
add_floor(J1X, Y2, R2Z + 500, 1000, 1000)

# ---- つづら折り坂 B1 → 踊り場 → B2 ----
_, sB1_y, sB1_z = add_slope(J1X, Y2, R2Z + 1000, 2000, 1000, "z+")
J2Z = sB1_z + 500
add_floor(J1X, sB1_y, J2Z, 1000, 1000)
sB2_x, sB2_y, _ = add_slope(J1X - 500, sB1_y, J2Z, 2000, 1000, "x-")
Y3 = sB2_y
J3X = sB2_x - 500
add_floor(J3X, Y3, J2Z, 1000, 1000)

# ---- ③ System32：クランク通路 ----
C1LEN = 2400.0
add_corridor(J3X, Y3, J2Z + 500 + C1LEN / 2, C1LEN, "z")
J4Z = J2Z + 500 + C1LEN + 500
add_floor(J3X, Y3, J4Z, 1000, 1000)
C2LEN = 2000.0
add_corridor(J3X - 500 - C2LEN / 2, Y3, J4Z, C2LEN, "x")
J5X = J3X - 500 - C2LEN - 500
add_floor(J5X, Y3, J4Z, 1000, 1000)
add_enemy("safari", J3X, Y3, J2Z + 1700)
add_enemy("xcode", J5X, Y3, J4Z + 1600)
add_light(J3X, Y3 + 800, J4Z, 5000, BLUE)
add_light(J5X, Y3 + 800, J4Z + 1800, 5000, BLUE)

# ---- 寄り道：Temp（行き止まり） ----
TEMP_X = J5X - 500 - 700 - 600
add_floor(J5X - 500 - 350, Y3, J4Z, 700, 800)
add_floor(TEMP_X, Y3, J4Z, 1200, 1200)
add_light(TEMP_X, Y3 + 600, J4Z, 2200, YELLOW)

# ---- C3（本ルート続き）→ 坂C ----
C3LEN = 2400.0
add_corridor(J5X, Y3, J4Z + 500 + C3LEN / 2, C3LEN, "z")
J6Z = J4Z + 500 + C3LEN + 500
add_floor(J5X, Y3, J6Z, 1000, 1000)
_, sC_y, sC_z = add_slope(J5X, Y3, J6Z + 500, 2600, 1000, "z+")

# ---- ④ Program Files（主戦場） ----
Y4 = sC_y
R4X = J5X
R4Z = sC_z + 1800
add_floor(R4X, Y4, R4Z, 3800, 3600)
add_window_wall(R4X - 1920, Y4, R4Z, 3600, 1500, 90)
add_window_wall(R4X + 1920, Y4, R4Z, 3600, 1500, -90)
for dx, dz in ((-900, -800), (900, -700), (-800, 900), (950, 850)):
    add_pillar(R4X + dx, Y4, R4Z + dz)
for dx, dz, br, lift in ((-1400, 100, 17, 0), (-1290, 240, -23, 0), (-1350, 170, 52, 220),
                         (1350, -200, 34, 0), (1420, -60, -40, 0), (200, 1300, 11, 0)):
    add_block(R4X + dx, Y4, R4Z + dz, br, lift=lift)
for ex, ez, kind in ((-1000, -400, "safari"), (1100, 300, "safari"), (-300, 1100, "safari"),
                     (400, -1100, "safari"), (500, -900, "xcode"), (-1300, 700, "xcode")):
    add_enemy(kind, R4X + ex, Y4, R4Z + ez)
add_light(R4X, Y4 + 1000, R4Z, 7000, BLUE)
add_light(R4X - 900, Y4 + 800, R4Z - 800, 2500, YELLOW)
add_light(R4X + 950, Y4 + 800, R4Z + 850, 2500, YELLOW)

# ---- UACゲート（区画の出口） ----
GATE_Z = R4Z + 1800
for side in (-1, 1):
    add_prop("wall_data", (R4X + side * (500 + 725), Y4 + 650, GATE_Z), (1450, 1300, 60))
add_light(R4X, Y4 + 1100, GATE_Z, 3000, YELLOW)

# ---- 坂D（青 → 白銀） ----
_, sD_y, sD_z = add_slope(R4X, Y4, GATE_Z + 100, 2600, 1000, "z+")

# ---- ⑤ Apple アリーナ ----
Y5 = sD_y
R5Z = sD_z + 1500
add_floor(R4X, Y5, R5Z, 3100, 3100, kind="floor_apple")
add_light(R4X, Y5 + 1600, R5Z, 8000, WHITE)
add_light(R4X, Y5 + 300, R5Z, 3000, RED)

stage = {
    "playerStart": {"position": [0.0, Y1, -900.0], "rotationY": 0.0},
    "props": props,
    "lights": lights,
    "spawns": spawns,
    "mac": {"type": "mac", "position": [R4X, round(Y5, 2), R5Z], "rotationY": 180.0},
}

out = sys.argv[1] if len(sys.argv) > 1 else "assets/data/stage-test.json"
with open(out, "w", encoding="utf-8") as f:
    json.dump(stage, f, ensure_ascii=False, indent=2)
    f.write("\n")

print(f"wrote {out}")
print(f"  props={len(props)} lights={len(lights)} spawns={len(spawns)}")
print(f"  depth: Desktop y={Y1:.0f} → Apple y={Y5:.0f}")
