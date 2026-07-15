"""
共通の FBX クリーンアップ処理：ボーン名除去・ウェイト制限・三角形化
"""

import bpy

PREFIX = "mixamorig:"


def strip_bone_prefix(src_path: str):
    """Blender で FBX を読み込み、ボーン名から mixamorig: プレフィックスを除去"""
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=src_path)

    for obj in bpy.data.objects:
        obj.name = obj.name.replace(PREFIX, "")
        if obj.type == 'ARMATURE':
            for bone in obj.data.bones:
                bone.name = bone.name.replace(PREFIX, "")

    print(f"Bone prefix stripped: {src_path}")


def limit_and_normalize_weights(max_weights: int = 4):
    """全メッシュのスキンウェイトを制限・正規化"""
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            bpy.context.view_layer.objects.active = obj
            bpy.ops.object.mode_set(mode='WEIGHT_PAINT')
            bpy.ops.object.vertex_group_limit_total(limit=max_weights)
            bpy.ops.object.vertex_group_normalize_all(lock_active=False)
            bpy.ops.object.mode_set(mode='OBJECT')

    print(f"Weights limited to {max_weights} per vertex and normalized")


def triangulate_meshes():
    """全メッシュを三角形化"""
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            bpy.context.view_layer.objects.active = obj
            mod = obj.modifiers.new(name="Triangulate", type='TRIANGULATE')
            bpy.ops.object.modifier_apply(modifier=mod.name)

    print("All meshes triangulated")


def export_fbx(out_path: str, object_types=None, bake_anim: bool = False, bake_anim_only: bool = False):
    """FBX をエクスポート

    Args:
        out_path: 出力ファイルパス
        object_types: エクスポート対象（デフォルト: MESH, ARMATURE）
        bake_anim: アニメーションをベイク
        bake_anim_only: TRUE の場合、メッシュを削除してアーマチュアのみをエクスポート
    """
    if object_types is None:
        object_types = {'ARMATURE', 'MESH'}

    if bake_anim_only:
        for obj in list(bpy.data.objects):
            if obj.type == 'MESH':
                bpy.data.objects.remove(obj, do_unlink=True)
        object_types = {'ARMATURE'}

    bpy.ops.export_scene.fbx(
        filepath=out_path,
        add_leaf_bones=False,
        bake_anim=bake_anim,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_simplify_factor=0.0,
        object_types=object_types,
    )
    print(f"Exported: {out_path}")
