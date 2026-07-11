"""
メッシュ分離スクリプト（Blender ヘッドレス実行用）

AI生成モデルなどで武器・小物がキャラ本体と同じメッシュに含まれている場合に、
つながっていないジオメトリ（ルースパーツ）ごとに分離して個別の FBX に出力する。

- 最大のパーツ = キャラ本体として <output>_body.fbx に出力
- 残りのパーツ = 大きい順に <output>_part01.fbx, _part02.fbx ... に出力

使い方:
  blender --background --factory-startup --python tools/separate_fbx.py -- \
    --input input.fbx --output-prefix cyber_suit

  パーツ一覧の確認だけ行う場合（出力なし）:
  blender --background --factory-startup --python tools/separate_fbx.py -- \
    --input input.fbx --list-only
"""

import sys
import argparse
import bpy


def separate_loose_parts():
    """全メッシュをルースパーツごとに分離する"""
    for obj in list(bpy.data.objects):
        if obj.type != 'MESH':
            continue
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.mesh.separate(type='LOOSE')
        bpy.ops.object.mode_set(mode='OBJECT')


def mesh_stats(obj):
    """メッシュの (ポリゴン数, バウンディングボックス寸法) を返す"""
    dims = obj.dimensions
    return len(obj.data.polygons), (dims.x, dims.y, dims.z)


def export_single(obj, path: str):
    """指定オブジェクトだけを FBX 出力する"""
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.ops.export_scene.fbx(
        filepath=path,
        use_selection=True,
        path_mode='COPY',
        embed_textures=True,
        add_leaf_bones=False,
        bake_anim=False,
    )
    print(f"EXPORT_DONE: {path}")


def main():
    parser = argparse.ArgumentParser(description="FBX をルースパーツごとに分離")
    parser.add_argument("--input", required=True, help="入力 FBX ファイルパス")
    parser.add_argument("--output-prefix", default=None,
                        help="出力ファイル名の接頭辞（例: cyber_suit）")
    parser.add_argument("--list-only", action="store_true",
                        help="分離結果の一覧表示のみ行い、出力しない")
    parser.add_argument("--min-faces", type=int, default=100,
                        help="この面数未満のパーツはゴミとみなして無視する")

    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)

    if not args.list_only and args.output_prefix is None:
        print("エラー: --output-prefix か --list-only のどちらかを指定してください")
        sys.exit(1)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=args.input)

    separate_loose_parts()

    # 面数の多い順に並べる（ゴミパーツは除外）
    meshes = [o for o in bpy.data.objects if o.type == 'MESH'
              and len(o.data.polygons) >= args.min_faces]
    meshes.sort(key=lambda o: len(o.data.polygons), reverse=True)

    print(f"\n=== 分離結果: {len(meshes)} パーツ ===")
    for i, obj in enumerate(meshes):
        faces, (dx, dy, dz) = mesh_stats(obj)
        label = "BODY(最大)" if i == 0 else f"part{i:02d}"
        print(f"  [{label}] {obj.name}: {faces:,} faces, size({dx:.2f}, {dy:.2f}, {dz:.2f})")

    if args.list_only:
        return

    # 最大パーツ = キャラ本体、残り = 個別パーツとして出力
    export_single(meshes[0], f"{args.output_prefix}_body.fbx")
    for i, obj in enumerate(meshes[1:], start=1):
        export_single(obj, f"{args.output_prefix}_part{i:02d}.fbx")


if __name__ == "__main__":
    main()
