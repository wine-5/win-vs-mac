"""
ポリゴン数削減スクリプト（Blender ヘッドレス実行用）

AI生成モデルなどの高ポリゴン FBX をゲーム用に削減する。
Mixamo へアップロードする前（リグ付け前）に実行すること。

使い方（全体を均一に削減）:
  blender --background --factory-startup --python tools/decimate_fbx.py -- \
    --input input.fbx --output output.fbx --target-faces 100000

使い方（頭部を高品質に残す2パス削減）:
  blender --background --factory-startup --python tools/decimate_fbx.py -- \
    --input input.fbx --output output.fbx \
    --protect-top 0.18 --head-faces 55000 --body-faces 85000

注意:
- 頭部保護モードはメッシュが1つのモデルを想定している
- Decimate の頂点グループ重みは「重い側から削り尽くす」挙動のため、
  比例配分はできない。そのため「片側を完全凍結して他方だけ目標まで削る」
  処理を2回行う方式を採用している
"""

import sys
import argparse
import bpy


def count_faces() -> int:
    """シーン内の全メッシュの三角形換算ポリゴン数を数える"""
    total = 0
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            for poly in obj.data.polygons:
                # 四角形以上は三角形換算（n角形 = n-2 三角形）
                total += max(len(poly.vertices) - 2, 1)
    return total


def count_split(obj, threshold_z):
    """しきい値より上（頭部）と下（体）の三角形換算ポリゴン数を数える"""
    head, body = 0, 0
    mw = obj.matrix_world
    for poly in obj.data.polygons:
        tris = max(len(poly.vertices) - 2, 1)
        if (mw @ obj.data.vertices[poly.vertices[0]].co).z >= threshold_z:
            head += tris
        else:
            body += tris
    return head, body


def head_vertex_indices(obj, threshold_z):
    """しきい値より上（頭部）の頂点インデックスを返す"""
    mw = obj.matrix_world
    return [v.index for v in obj.data.vertices if (mw @ v.co).z >= threshold_z]


def apply_decimate(obj, ratio: float, group_name: str = "", invert: bool = False):
    """Decimate（Collapse）を適用する。group_name 指定時は対象を頂点グループで制御"""
    bpy.context.view_layer.objects.active = obj
    mod = obj.modifiers.new(name="Decimate", type='DECIMATE')
    mod.decimate_type = 'COLLAPSE'
    mod.ratio = ratio
    mod.use_collapse_triangulate = True
    if group_name:
        mod.vertex_group = group_name
        mod.invert_vertex_group = invert
        mod.vertex_group_factor = 1.0
    bpy.ops.object.modifier_apply(modifier=mod.name)


def decimate_uniform(ratio: float):
    """全メッシュを均一な比率で削減する"""
    for obj in bpy.data.objects:
        if obj.type != 'MESH':
            continue
        apply_decimate(obj, ratio)
        print(f"  decimated: {obj.name} -> {len(obj.data.polygons)} faces")


def decimate_with_head_protection(protect_top: float, head_faces: int, body_faces: int):
    """頭部と体を別々の目標ポリゴン数まで削減する（2パス方式）

    パス1: 頭部を完全凍結して体だけを body_faces まで削減
    パス2: 体を完全凍結して頭部だけを head_faces まで削減
    """
    obj = next(o for o in bpy.data.objects if o.type == 'MESH')

    coords_z = [(obj.matrix_world @ v.co).z for v in obj.data.vertices]
    z_min, z_max = min(coords_z), max(coords_z)
    threshold = z_max - (z_max - z_min) * protect_top

    h0, b0 = count_split(obj, threshold)
    print(f"  before: head {h0:,}, body {b0:,} (threshold z >= {threshold:.3f})")

    # パス1: 頭を凍結（重み1.0 + invert = 頭は削減対象外）→ 体だけ削減
    group1 = obj.vertex_groups.new(name="HeadFreeze")
    group1.add(head_vertex_indices(obj, threshold), 1.0, 'REPLACE')
    apply_decimate(obj, (h0 + body_faces) / (h0 + b0), "HeadFreeze", invert=True)

    h1, b1 = count_split(obj, threshold)
    print(f"  pass1 (body): head {h1:,}, body {b1:,}")

    # パス2: 体を凍結（頭に重み1.0・invertなし = 頭だけ削減対象）
    group2 = obj.vertex_groups.new(name="HeadOnly")
    group2.add(head_vertex_indices(obj, threshold), 1.0, 'REPLACE')
    apply_decimate(obj, (b1 + head_faces) / (h1 + b1), "HeadOnly", invert=False)

    h2, b2 = count_split(obj, threshold)
    print(f"  pass2 (head): head {h2:,}, body {b2:,}")

    # 作業用の頂点グループを削除
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)


def main():
    parser = argparse.ArgumentParser(description="FBX のポリゴン数を削減")
    parser.add_argument("--input", required=True, help="入力 FBX ファイルパス")
    parser.add_argument("--output", required=True, help="出力 FBX ファイルパス")
    parser.add_argument("--ratio", type=float, default=None,
                        help="削減比率（0.0-1.0）。0.05 = 5%%まで削減")
    parser.add_argument("--target-faces", type=int, default=None,
                        help="目標ポリゴン数（例: 100000）。ratio より優先")
    parser.add_argument("--protect-top", type=float, default=0.0,
                        help="頭部として保護する高さの割合（例: 0.18 = 上から18%%）")
    parser.add_argument("--head-faces", type=int, default=55000,
                        help="頭部の目標ポリゴン数（--protect-top 指定時のみ使用）")
    parser.add_argument("--body-faces", type=int, default=85000,
                        help="体の目標ポリゴン数（--protect-top 指定時のみ使用）")

    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)

    is_protect_mode = args.protect_top > 0.0
    if not is_protect_mode and args.ratio is None and args.target_faces is None:
        print("エラー: --ratio / --target-faces / --protect-top のいずれかを指定してください")
        sys.exit(1)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=args.input)

    before = count_faces()
    print(f"\n削減前: {before:,} faces")

    if is_protect_mode:
        decimate_with_head_protection(args.protect_top, args.head_faces, args.body_faces)
    else:
        ratio = args.ratio
        if args.target_faces is not None:
            ratio = min(args.target_faces / before, 1.0)
            print(f"目標 {args.target_faces:,} faces -> ratio = {ratio:.4f}")
        decimate_uniform(ratio)

    after = count_faces()
    print(f"削減後: {after:,} faces（{after / before * 100:.1f}%）\n")

    bpy.ops.export_scene.fbx(
        filepath=args.output,
        path_mode='COPY',
        embed_textures=True,
        add_leaf_bones=False,
        bake_anim=False,
    )
    print(f"EXPORT_DONE: {args.output}")


if __name__ == "__main__":
    main()
