# Mixamo → DxLib mv1 変換ツール

Mixamo からダウンロードした FBX を DxLib で使用可能な状態に変換します。

## 前提条件

- Blender 4.3 以上がインストール済み
- PATH に `blender` コマンドが登録されている

## ファイル構成

- `convert_fbx.py` — メイン実行スクリプト（これを実行する）
- `blender_fbx_cleanup.py` — ボーン名除去・ウェイト制限・三角形化の共通処理
- `extract_anim_only.py` — アニメーション抽出（メッシュ削除・フレーム範囲調整）
- `decimate_fbx.py` — ポリゴン数削減（AI生成モデルなどの高ポリモデルをゲーム用に軽量化）
- `separate_fbx.py` — メッシュ分離（キャラ本体と武器・小物が一体化しているモデルをパーツごとに分割）

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

### 高ポリゴンモデルの削減（Mixamo アップロード前に実行）

```bash
blender --background --factory-startup --python tools/decimate_fbx.py -- \
  --input "futuristic cyber suit 3d model.fbx" \
  --output cyber_suit_lowpoly.fbx \
  --target-faces 100000
```

- `--target-faces`: 目標ポリゴン数（ゲーム用キャラは 5万〜15万 が目安）
- `--ratio`: 比率で直接指定する場合（例: `--ratio 0.05` = 5% まで削減）
- スキンウェイト・UV は保持されるが、**リグ付け前（Mixamo アップロード前）に実行すること**

顔のディテールを残したい場合（頭部と体を別々の目標値で削減）:

```bash
blender --background --factory-startup --python tools/decimate_fbx.py -- \
  --input model.fbx --output model_lowpoly.fbx \
  --protect-top 0.18 --head-faces 55000 --body-faces 85000
```

- `--protect-top 0.18`: モデルの上から18%を「頭部」として扱う
- 頭部→`--head-faces`、体→`--body-faces` の目標値までそれぞれ削減される（2パス方式）

### 武器・小物の分離（キャラと一体化している場合）

```bash
# まず一覧確認（何が何パーツあるか見る）
blender --background --factory-startup --python tools/separate_fbx.py -- \
  --input model.fbx --list-only

# 分離して出力（最大パーツ = _body.fbx、残り = _part01.fbx ...）
blender --background --factory-startup --python tools/separate_fbx.py -- \
  --input model.fbx --output-prefix cyber_suit
```

- つながっていないジオメトリ（ルースパーツ）単位で分離する
- `_body.fbx` を Mixamo に上げる。武器パーツは将来ボーン追従の装備品として利用可能

AI生成モデルを使う場合の全体フロー:
`AI生成 FBX → separate_fbx.py で本体と武器を分離 → decimate_fbx.py で削減 → Mixamo でオートリグ → convert_fbx.py → DxLibModelViewer で mv1`

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
