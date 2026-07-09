# Mixamo → DxLib mv1 変換ツール

Mixamo からダウンロードした FBX を DxLib で使用可能な状態に変換します。

## 前提条件

- Blender 4.3 以上がインストール済み
- PATH に `blender` コマンドが登録されている

## ファイル構成

- `convert_fbx.py` — メイン実行スクリプト（これを実行する）
- `blender_fbx_cleanup.py` — ボーン名除去・ウェイト制限・三角形化の共通処理
- `extract_anim_only.py` — アニメーション抽出（メッシュ削除・フレーム範囲調整）

## 使い方

### モデル用（メッシュ + アーマチュア）の変換

```bash
blender --background --factory-startup --python tools/convert_fbx.py -- \
  --input Downloads/Player.fbx \
  --output Player_model.fbx
```

DxLibModelViewer で `Player_model.fbx` を開いて → mv1 として保存

### アニメーション用（アーマチュアのみ）の変換

```bash
blender --background --factory-startup --python tools/convert_fbx.py -- \
  --input Downloads/Player_idle.fbx \
  --output Player_idle_anim.fbx \
  --anim-only
```

DxLibModelViewer で `Player_idle_anim.fbx` を開いて → mv1 として保存

## 処理の流れ

1. **Mixamo から FBX ダウンロード**
   - 対象モデル：オートリガー適用済み（Standard 65 bones）
   - Format: FBX Binary
   - Skin: モデルなら With Skin、アニメーションなら Without Skin

2. **このスクリプトで処理**
   - ボーン名から `mixamorig:` プレフィックスを除去
   - スキンウェイトを頂点あたり4本に制限＋正規化
   - メッシュを三角形化
   - （アニメーション専用の場合）メッシュ削除 & フレーム範囲自動調整

3. **DxLibModelViewer で mv1 変換**
   - 64bit 版を起動
   - File → Open で FBX を開く
   - 動きが正常か確認
   - File → Save As で mv1 形式保存

## トラブルシューティング

### DxLibModelViewer がクラッシュする

原因: ウェイト制限されていないメッシュ
→ このスクリプトで処理するか、別のモデルを選ぶ

### アニメーションに余白がある（30フレーム実データ + 220フレーム停止）

原因: Blender のシーンフレーム範囲がアクションの実範囲より広い
→ `--anim-only` オプションを使用（自動調整される）

### テクスチャが見えない

原因: 出力 FBX と同階層に `.fbm` フォルダ（テクスチャ）がない
→ DxLibModelViewer で `Without Skin` でエクスポートした場合は、元の FBX の `.fbm` フォルダを同じフォルダにコピー
