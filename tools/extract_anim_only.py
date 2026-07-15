"""
アニメーション抽出処理：メッシュ削除・実フレーム範囲への自動調整
"""

import bpy


def set_scene_range_to_action():
    """シーンの frame_start / frame_end をアクションの実フレーム範囲に合わせる

    これにより、余分なフレームがベイク込みされるのを防ぐ
    """
    for action in bpy.data.actions:
        start, end = action.frame_range
        bpy.context.scene.frame_start = int(start)
        bpy.context.scene.frame_end = int(end)
        print(f"ACTION_RANGE: {action.name} frames {int(start)}-{int(end)}")


def remove_meshes():
    """全メッシュを削除（アーマチュア・アニメーションのみ残す）"""
    for obj in list(bpy.data.objects):
        if obj.type == 'MESH':
            bpy.data.objects.remove(obj, do_unlink=True)

    print("Meshes removed; armature and animations retained")
