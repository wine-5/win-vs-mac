#!/usr/bin/env python3
"""ブロックをあらかじめ割った状態のモデル(.mqo)を生成する。

破壊演出で「同じ形の小さな立方体が飛び散る」のを避けるためのツール。
1個のブロックを不揃いな多面体へ分割し、破片1つを1オブジェクトとして書き出す。
mqoのオブジェクトはmv1へ変換するとフレームになるため、ゲーム側は
フレームごとに座標変換を差し込むだけで破片を飛ばせる。

分割の方法:
    立方体の内部に格子状の点を置き、外周以外の点をランダムにずらす。
    その格子の各マスを1個の破片（六面体）とする。点は隣り合う破片で共有するので、
    ずらしても破片同士に隙間ができず、組み上げると元の立方体に戻る。

UVの当て方:
    外側の面には元のブロックと同じUVを面ごとに平行投影で貼る。
    こうすると破片が組み上がった状態では元のブロックと同じ絵に見え、
    バラけると破片ごとに絵の違う部分が乗る。
    内側の面（破断面）は別マテリアルにして、テクスチャを貼らず暗い色にする。

出力（assets/model/stage/ 配下）:
    <Name>Fractured.mqo … 破片をオブジェクトに分けた立方体

使い方（リポジトリのルートで実行）:
    python tools/gen_fracture_models.py
    python tools/gen_fracture_models.py BlockZip
"""
import os
import random
import sys
import zlib

OUT_DIR = os.path.join("assets", "model", "stage")

# 立方体の一辺（gen_stage_models.py と揃える。これを実寸として扱う）
CUBE_SIZE = 100.0

# 1辺あたりの分割数。DIVISION^3 個の破片になる
DIVISION = 3

# 格子点をずらす量（マスの大きさに対する比率）。大きいほど破片が不揃いになる
JITTER = 0.34

# シードの土台。ブロック名と混ぜてブロックごとに違う割れ方にする
SEED_BASE = 20260731

# 破断面の色（テクスチャを貼らないマテリアル）
INNER_COLOR = (0.10, 0.12, 0.16)

# 既定の生成対象
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
    "BlockRam",
]

# 六面体の各面を (どの角を使うか, 面の向き) で表す。
# 角は (di, dj, dk) の 0/1 で指定し、外から見て左回りになる順に並べる。
# 巻き方向を間違えると裏返って描画されないため、gen_stage_models.py と同じ規約に従う。
HEX_FACES = [
    ([(1, 0, 1), (1, 0, 0), (1, 1, 0), (1, 1, 1)], "+X"),
    ([(0, 0, 0), (0, 0, 1), (0, 1, 1), (0, 1, 0)], "-X"),
    ([(0, 1, 1), (1, 1, 1), (1, 1, 0), (0, 1, 0)], "+Y"),
    ([(0, 0, 0), (1, 0, 0), (1, 0, 1), (0, 0, 1)], "-Y"),
    ([(0, 0, 1), (1, 0, 1), (1, 1, 1), (0, 1, 1)], "+Z"),
    ([(0, 0, 0), (0, 1, 0), (1, 1, 0), (1, 0, 0)], "-Z"),
]

# 面の向きごとに、UVへ使う軸と符号。
# 元のブロックは6面すべてに同じ絵を正立で貼っているので、それに合わせる
UV_AXES = {
    "+X": ("z", "y", -1, -1),
    "-X": ("z", "y", 1, -1),
    "+Y": ("x", "z", -1, -1),
    "-Y": ("x", "z", -1, 1),
    "+Z": ("x", "y", 1, -1),
    "-Z": ("x", "y", -1, -1),
}


def make_rng(name):
    """ブロック名から乱数エンジンを作る（同じ名前なら毎回同じ割れ方）"""
    return random.Random(SEED_BASE ^ zlib.crc32(name.encode("utf-8")))


def build_lattice(rng):
    """格子点を作る。外周の点はその軸方向へずらさない（外形を保つため）"""
    half = CUBE_SIZE / 2.0
    step = CUBE_SIZE / DIVISION
    lattice = {}
    for i in range(DIVISION + 1):
        for j in range(DIVISION + 1):
            for k in range(DIVISION + 1):
                x = -half + i * step
                y = -half + j * step
                z = -half + k * step
                if 0 < i < DIVISION:
                    x += rng.uniform(-1, 1) * step * JITTER
                if 0 < j < DIVISION:
                    y += rng.uniform(-1, 1) * step * JITTER
                if 0 < k < DIVISION:
                    z += rng.uniform(-1, 1) * step * JITTER
                lattice[(i, j, k)] = (x, y, z)
    return lattice


def face_uv(vertex, orientation):
    """面の向きに応じて、頂点座標からUVを平行投影で求める"""
    axis_u, axis_v, sign_u, sign_v = UV_AXES[orientation]
    index = {"x": 0, "y": 1, "z": 2}
    half = CUBE_SIZE / 2.0

    # -half〜+half を 0〜1 へ写す
    u = (vertex[index[axis_u]] * sign_u + half) / CUBE_SIZE
    v = (vertex[index[axis_v]] * sign_v + half) / CUBE_SIZE
    return (u, v)


def is_outer(orientation, i, j, k):
    """その面が立方体の外側に露出しているか"""
    return {
        "+X": i == DIVISION - 1,
        "-X": i == 0,
        "+Y": j == DIVISION - 1,
        "-Y": j == 0,
        "+Z": k == DIVISION - 1,
        "-Z": k == 0,
    }[orientation]


def build_shard(lattice, i, j, k):
    """1個の破片（六面体）の頂点とUV付きの面を返す"""
    vertices = []
    lookup = {}

    def corner(di, dj, dk):
        key = (di, dj, dk)
        if key not in lookup:
            lookup[key] = len(vertices)
            vertices.append(lattice[(i + di, j + dj, k + dk)])
        return lookup[key]

    faces = []
    for corners, orientation in HEX_FACES:
        indices = [corner(*c) for c in corners]
        outer = is_outer(orientation, i, j, k)
        uvs = [face_uv(vertices[n], orientation) for n in indices]
        faces.append((indices, uvs, 0 if outer else 1))
    return vertices, faces


def mqo_text(texture_filename, shards):
    """破片オブジェクトを並べたmqoテキストを返す"""
    lines = [
        "Metasequoia Document",
        "Format Text Ver 1.0",
        "",
        "Material 2 {",
        '\t"outer" shader(3) col(1.000 1.000 1.000 1.000) dif(1.000) '
        'amb(1.000) emi(0.000) spc(0.000) power(5.00) tex("%s")' % texture_filename,
        '\t"inner" shader(3) col(%.3f %.3f %.3f 1.000) dif(1.000) '
        'amb(1.000) emi(0.000) spc(0.000) power(5.00)' % INNER_COLOR,
        "}",
    ]

    for index, (vertices, faces) in enumerate(shards):
        lines.append('Object "shard_%02d" {' % index)
        lines.append("\tvisible 15")
        lines.append("\tlocking 0")
        lines.append("\tshading 1")
        lines.append("\tcolor 0.6 0.6 0.6")
        lines.append("\tcolor_type 0")
        lines.append("\tvertex %d {" % len(vertices))
        for v in vertices:
            lines.append("\t\t%.4f %.4f %.4f" % v)
        lines.append("\t}")
        lines.append("\tface %d {" % len(faces))
        for indices, uvs, material in faces:
            uv_text = " ".join("%.4f %.4f" % uv for uv in uvs)
            lines.append("\t\t4 V(%d %d %d %d) M(%d) UV(%s)"
                         % (indices[0], indices[1], indices[2], indices[3], material, uv_text))
        lines.append("\t}")
        lines.append("}")

    return "\n".join(lines) + "\n"


def generate(name):
    texture = f"{name}.png"
    if not os.path.exists(os.path.join(OUT_DIR, texture)):
        print(f"  スキップ: {texture} が見つかりません")
        return

    rng = make_rng(name)
    lattice = build_lattice(rng)

    shards = []
    for i in range(DIVISION):
        for j in range(DIVISION):
            for k in range(DIVISION):
                shards.append(build_shard(lattice, i, j, k))

    output = os.path.join(OUT_DIR, f"{name}Fractured.mqo")
    with open(output, "w", encoding="utf-8") as f:
        f.write(mqo_text(texture, shards))
    print(f"  生成: {output}（破片 {len(shards)} 個）")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    targets = sys.argv[1:] or DEFAULT_TARGETS
    for name in targets:
        print(f"{name}:")
        generate(name)


if __name__ == "__main__":
    main()
