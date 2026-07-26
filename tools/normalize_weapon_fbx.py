"""武器モデル正規化スクリプト（Blender ヘッドレス実行用）

AI生成（Tripo等）の武器 FBX を、ゲームへ持ち込める形へ一括で整える。
mv1 へ変換する前に必ず実行すること（原点と軸は mv1 化した後では直せない）。

やること:
  1. 主成分分析で刃の長軸を求め、刃を +Z（書き出し後は +Y）へ向ける
  2. 断面の太さから鍔と柄を判定し、原点を握りの中心へ移す
  3. 全長を指定の長さへスケールする
  4. ポリゴン数を目標まで削減する
  5. テクスチャを指定サイズへ縮小する

AI生成モデルは元画像の傾きをそのまま再現することがあり、その場合
バウンディングボックスの軸と刃の向きは一致しない。そのため軸の判定には
AABB ではなく主成分分析を使っている。

使い方:
  blender --background --factory-startup --python tools/normalize_weapon_fbx.py -- \
    --input input.fbx --output PlayerSword.fbx \
    --total-length 0.9 --target-tris 12000 --texture-size 1024

握りの位置が合わない場合は --grip-shift で長軸方向へずらして調整する
（正の値で柄頭寄り、負の値で鍔寄り。単位は正規化後の長さ）。
"""

import sys
import argparse

import numpy as np
import bpy
import mathutils

# 長軸を分割して断面の太さを測る区間数。鍔（最も太い区間）の位置を
# 特定できる程度に細かく、かつノイズを拾わない程度に粗くする
SEGMENT_COUNT = 24


def get_mesh_objects():
    """シーン内のメッシュオブジェクトを返す"""
    return [o for o in bpy.data.objects if o.type == 'MESH']


def count_tris() -> int:
    """シーン内の全メッシュの三角形換算ポリゴン数を数える"""
    total = 0
    for obj in get_mesh_objects():
        for poly in obj.data.polygons:
            total += max(len(poly.vertices) - 2, 1)
    return total


def bake_world_transform(obj):
    """オブジェクトのワールド変換をメッシュへ焼き込み、変換を単位行列に戻す"""
    obj.data.transform(obj.matrix_world)
    obj.matrix_world = mathutils.Matrix.Identity(4)


def compute_principal_axes(points):
    """頂点群の主成分（長軸・幅・厚み）の方向を求める

    :param points: 頂点座標の配列
    :return: (中心, 主成分方向を列に持つ3x3行列)
    """
    center = points.mean(axis=0)
    cov = np.cov((points - center).T)
    values, vectors = np.linalg.eigh(cov)
    order = np.argsort(values)[::-1]
    return center, vectors[:, order]


def measure_segments(along, width1, width2):
    """長軸を等分し、区間ごとの断面の太さ（幅×厚み）を測る

    :param along: 長軸方向の座標
    :param width1: 幅方向の座標
    :param width2: 厚み方向の座標
    :return: 区間ごとの (下限, 上限, 断面積) のリスト
    """
    low, high = along.min(), along.max()
    step = (high - low) / SEGMENT_COUNT
    segments = []
    for i in range(SEGMENT_COUNT):
        a0, a1 = low + step * i, low + step * (i + 1)
        mask = (along >= a0) & (along <= a1)
        if not mask.any():
            segments.append((a0, a1, 0.0))
            continue
        w1 = width1[mask].max() - width1[mask].min()
        w2 = width2[mask].max() - width2[mask].min()
        segments.append((a0, a1, w1 * w2))
    return segments


def is_blade_at_min(segments) -> bool:
    """長軸の最小側が刃先かどうかを断面積から判定する

    刃は細長く、柄は握れる太さがある。両端 1/4 の平均断面積を比べ、
    細い側を刃先とみなす
    """
    quarter = max(SEGMENT_COUNT // 4, 1)
    near_min = np.mean([s[2] for s in segments[:quarter]])
    near_max = np.mean([s[2] for s in segments[-quarter:]])
    return near_min < near_max


def find_grip_center(segments, blade_at_min: bool) -> float:
    """握りの中心（長軸方向の座標）を求める

    最も断面積の大きい区間を鍔とみなし、そこから柄側の領域の中点を返す
    """
    guard_index = int(np.argmax([s[2] for s in segments]))
    if blade_at_min:
        # 刃が最小側 = 柄は最大側。鍔より先（最大側）が柄
        grip = segments[guard_index + 1:]
    else:
        # 刃が最大側 = 柄は最小側。鍔より手前（最小側）が柄
        grip = segments[:guard_index]
    if not grip:
        # 鍔が端にあり柄を切り出せない場合は全体の中点で妥協する
        return (segments[0][0] + segments[-1][1]) * 0.5
    return (grip[0][0] + grip[-1][1]) * 0.5


def normalize_orientation(obj, total_length: float, grip_shift: float):
    """刃を +Z へ向け、原点を握りの中心へ置き、全長を揃える"""
    bake_world_transform(obj)
    points = np.array([v.co[:] for v in obj.data.vertices])

    center, axes = compute_principal_axes(points)
    projected = (points - center) @ axes
    segments = measure_segments(projected[:, 0], projected[:, 1], projected[:, 2])

    blade_at_min = is_blade_at_min(segments)
    grip_center = find_grip_center(segments, blade_at_min)
    length = projected[:, 0].max() - projected[:, 0].min()
    print(f"  長軸の全長: {length:.4f}")
    print(f"  刃先は長軸の{'最小' if blade_at_min else '最大'}側")
    print(f"  握りの中心（長軸座標）: {grip_center:.4f}")

    # 主成分を軸へ割り当てる。長軸→Z、幅→X、厚み→Y。
    # 刃が最小側にある場合は長軸を反転させ、必ず刃が +Z を向くようにする
    long_axis = axes[:, 0] * (-1.0 if blade_at_min else 1.0)
    width_axis = axes[:, 1]
    thickness_axis = np.cross(long_axis, width_axis)
    rotation = mathutils.Matrix(
        [list(width_axis), list(thickness_axis), list(long_axis)]
    ).to_4x4()

    # 握りの中心を原点へ。長軸を反転した場合は握りの座標も符号が反転する
    grip_along = -grip_center if blade_at_min else grip_center
    scale = total_length / length

    obj.data.transform(mathutils.Matrix.Translation(-mathutils.Vector(center.tolist())))
    obj.data.transform(rotation)
    obj.data.transform(mathutils.Matrix.Translation(mathutils.Vector((0.0, 0.0, -grip_along))))
    obj.data.transform(mathutils.Matrix.Diagonal((scale, scale, scale, 1.0)))
    if grip_shift != 0.0:
        obj.data.transform(mathutils.Matrix.Translation(mathutils.Vector((0.0, 0.0, -grip_shift))))

    print(f"  スケール倍率: {scale:.5f}（全長 {total_length}）")


def decimate(target_tris: int):
    """全メッシュを目標ポリゴン数まで削減する"""
    before = count_tris()
    if before <= target_tris:
        print(f"  削減不要（{before:,} tris）")
        return

    ratio = target_tris / before
    for obj in get_mesh_objects():
        bpy.context.view_layer.objects.active = obj
        modifier = obj.modifiers.new(name="Decimate", type='DECIMATE')
        modifier.decimate_type = 'COLLAPSE'
        modifier.ratio = ratio
        modifier.use_collapse_triangulate = True
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    print(f"  ポリゴン削減: {before:,} -> {count_tris():,} tris（ratio {ratio:.5f}）")


def drop_unused_textures(keep_keywords):
    """指定のキーワードを名前に含まないテクスチャをマテリアルから外す

    DxLib の mv1 が扱えるのはディフューズとノーマルマップまでで、
    metallic や roughness を持たせても描画に使われないままメモリを占める
    """
    dropped = []
    for material in bpy.data.materials:
        if not material.use_nodes:
            continue
        for node in list(material.node_tree.nodes):
            if node.type != 'TEX_IMAGE' or node.image is None:
                continue
            name = node.image.name.lower()
            if any(keyword in name for keyword in keep_keywords):
                continue
            dropped.append(node.image.name)
            material.node_tree.nodes.remove(node)

    for image in list(bpy.data.images):
        if image.users == 0:
            bpy.data.images.remove(image)

    for name in dropped:
        print(f"  テクスチャ除外: {name}")


def resize_textures(size: int):
    """テクスチャを指定サイズ以下へ縮小し、FBXへ埋め込めるようパックする"""
    for image in bpy.data.images:
        if image.size[0] <= 0 or image.size[0] <= size:
            continue
        original = f"{image.size[0]}x{image.size[1]}"
        image.scale(size, size)
        image.pack()
        print(f"  テクスチャ縮小: {image.name} {original} -> {size}x{size}")


def main():
    parser = argparse.ArgumentParser(description="武器FBXをゲーム用に正規化する")
    parser.add_argument("--input", required=True, help="入力 FBX ファイルパス")
    parser.add_argument("--output", required=True, help="出力 FBX ファイルパス")
    parser.add_argument("--total-length", type=float, default=0.9,
                        help="正規化後の全長（Blender単位）")
    parser.add_argument("--target-tris", type=int, default=12000,
                        help="目標三角形数")
    parser.add_argument("--texture-size", type=int, default=1024,
                        help="テクスチャの最大辺")
    parser.add_argument("--grip-shift", type=float, default=0.0,
                        help="握り位置の微調整（正で柄頭寄り、負で鍔寄り）")
    parser.add_argument("--keep-textures", default="",
                        help="残すテクスチャ名のキーワード（カンマ区切り。例: basecolor,normal）"
                             "。未指定なら全て残す")

    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=args.input)

    meshes = get_mesh_objects()
    if not meshes:
        print("エラー: メッシュが見つかりません")
        sys.exit(1)

    print(f"\n読み込み: {len(meshes)} メッシュ / {count_tris():,} tris")

    print("[1/3] 向き・原点・スケールの正規化")
    for obj in meshes:
        normalize_orientation(obj, args.total_length, args.grip_shift)

    print("[2/3] ポリゴン削減")
    decimate(args.target_tris)

    print("[3/3] テクスチャ整理")
    if args.keep_textures:
        drop_unused_textures([k.strip().lower() for k in args.keep_textures.split(",") if k.strip()])
    resize_textures(args.texture_size)

    # axis_up='Y' で書き出すことで、Blenderの +Z（刃の向き）が FBX の +Y になる
    bpy.ops.export_scene.fbx(
        filepath=args.output,
        path_mode='COPY',
        embed_textures=True,
        add_leaf_bones=False,
        bake_anim=False,
        axis_forward='-Z',
        axis_up='Y',
    )
    print(f"\nEXPORT_DONE: {args.output}")


if __name__ == "__main__":
    main()
