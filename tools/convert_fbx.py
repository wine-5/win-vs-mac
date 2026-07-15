#!/usr/bin/env python3
"""
Mixamo FBX → DxLib mv1 用 FBX 変換スクリプト

使い方 1（Blender ヘッドレス実行）:
  blender --background --factory-startup --python tools/convert_fbx.py -- \\
    --input input.fbx --output output.fbx [--anim-only]

使い方 2（直接 Python 実行 - Blender 内から）:
  python tools/convert_fbx.py --input input.fbx --output output.fbx [--anim-only]
"""

import os
import sys
import argparse
import bpy

# tools フォルダをパスに追加（モジュールインポート用）
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import blender_fbx_cleanup as cleanup
import extract_anim_only as extract


def main(args=None):
    """メイン処理"""
    parser = argparse.ArgumentParser(
        description="Mixamo FBX を DxLib mv1 用に変換"
    )
    parser.add_argument("--input", required=True, help="入力 FBX ファイルパス")
    parser.add_argument("--output", required=True, help="出力 FBX ファイルパス")
    parser.add_argument(
        "--anim-only",
        action="store_true",
        help="TRUE: アニメーション専用（メッシュ削除）。FALSE: モデル用（メッシュ＋アーマチュア）"
    )

    # Blender ヘッドレス実行時は sys.argv の処理が異なる
    if "--" in sys.argv:
        argv = sys.argv[sys.argv.index("--") + 1:]
        parsed = parser.parse_args(argv)
    else:
        parsed = parser.parse_args(args)

    print(f"\n=== FBX Conversion ===")
    print(f"Input:  {parsed.input}")
    print(f"Output: {parsed.output}")
    print(f"Mode:   {'Animation-only' if parsed.anim_only else 'Model (mesh + armature)'}\n")

    try:
        # 1. ボーン名から mixamorig: を除去
        cleanup.strip_bone_prefix(parsed.input)

        # 2. ウェイト制限・正規化・三角形化（常に実施）
        cleanup.limit_and_normalize_weights(max_weights=4)
        cleanup.triangulate_meshes()

        # 3. アニメーション専用の場合：メッシュ削除 & フレーム範囲設定
        if parsed.anim_only:
            extract.remove_meshes()
            extract.set_scene_range_to_action()
            cleanup.export_fbx(parsed.output, bake_anim=True, bake_anim_only=True)
        else:
            # モデル用：アニメーションなし
            cleanup.export_fbx(parsed.output, bake_anim=False, bake_anim_only=False)

        print(f"\n✓ 変換完了: {parsed.output}")

    except Exception as e:
        print(f"\n✗ エラー: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
