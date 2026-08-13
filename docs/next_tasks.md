# 次にやること（引き継ぎ）

最終更新: 2026-08-13 / ブランチ `feature/item`

拡張子の「壊す → 拾う → 効く → 付け替える」ループと、ギャンブルボックス（当たりで能力2倍）
まで実装が終わり、レベルデザインも一段落した。
**残っている大きな作業はメモリ削減ひとつ。** PRを出す前にこれを通したい。

---

## 0. 先に読むもの

| ファイル | 内容 |
|---|---|
| [CLAUDE.md](../CLAUDE.md) | アーキテクチャ・命名・コミット規約。**最優先** |
| [docs/architecture/architecture.md](architecture/architecture.md) | レイヤー構成 |
| [docs/conventions/naming_convention.md](conventions/naming_convention.md) | 命名の詳細 |

### 作業前に必ず守ること

- **ビルド**：`MSBuild.exe DxLib-3D.vcxproj /p:Configuration=Debug /p:Platform=x64`
  （MSBuildは `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\`）
  ゲームが起動したままだと `LNK1168` になる。**先に閉じる**
- **コミット**：1コミット＝1つの小さな部品。実装 → 即コミット → 次、の順で進める。
  メッセージは日本語一行の Conventional Commits。`push` は絶対にしない
- **ツールと生成物は別コミット**にする（`tools/gen_*.py` と `assets/` を混ぜない）
- `git add` は自分が触ったファイルだけを明示列挙する。`git add -A` は使わない
- **エディタ（`tools/stage-editor/`）を開いたままステージを編集しない。**
  タブを開いた時点の状態が保存されるので、その間にJSONを直すと書き戻される
- **見た目に触ったらスクリーンショットを撮って自分の目で確かめる。**
  数値が良くなっても絵が壊れていることがある（実際に、モデルを軽くしたら
  テクスチャ参照が切れて全キャラが真っ白になった）。変更前後を同じ時刻で撮って比べる

---

## 1. メモリ削減（メインタスク）

**着手時 1,858MB → 現在 1,488MB → 目標 1,000MB以下**（Release・プライベートバイト実測）。

### 測り方

数値を推測で語らない。`WRITE_DEBUG_LOG_FILES` を `true` にして Release をビルドすると
`memory_probe.txt` に「計測地点・使用量・直前からの増分」が出る
（[DebugFlags.h](../src/core/constant/DebugFlags.h)。プローブは stash ではなく本流に入っている）。
細かく見たいときは `Application.cpp` の `PROBE_INTERVAL_FRAMES` を小さくする。

- **Debug は Release の約2倍**（1,858MB のとき Debug は同条件で計測していない）。**必ず Release で測る**
- プローブを有効にすると 10MB 程度増える。無効のまま外から測るなら
  `Get-Process ... PrivateMemorySize64` を1秒ごとに取ればよい
- 起動〜20秒でプラトーに達する。リークは無い

### 実測の内訳（2026-08-13・25kモデル適用前）

| 区間 | 増分 |
|---|---|
| `enemy_xcode` 初回展開（140,172ポリゴン） | +243MB |
| `InGame: loadResources`（`Player.mv1` 140,210ポリゴン） | +225MB |
| `DxLib_Init` | +155MB（動かせない） |
| `spawn: initializeProps`（配置物79個。**ほぼブロックのテクスチャ**） | +187MB → 対策後 129MB |
| `frame 60`（`Mac.mv1` 100,000ポリゴン＋初回描画） | +108MB |
| `enemy_safari` 初回展開（100,000ポリゴン） | +103MB |
| フォント最初の1つ（DxLibのフォント基盤の初期化込み） | +67MB |
| `EffectFactory`（`Effekseer_Init(8000)`） | +52MB |
| `spawn: initializePlayer`（`PlayerSword.mv1`） | +50MB |
| タブ3種 | +41MB |
| `AudioManager` | +38MB |
| フォント2つ目以降（1つあたり約4MB） | +40MB |

**主因はテクスチャではなくモデルの展開量**（キャラ4体で約670MB）。

### 否定された前提（蒸し返さない）

| 説 | 実測 |
|---|---|
| 画像212枚・展開1,080MBが主因 | **誤り**。常駐しているモデルテクスチャは合計約84MB。`dumpModelStats` の `texMB` で確認できる |
| `Player.fbm` / `PlayerModelTexture.fbm` の1024px群が効いている | **誤り**。どちらも実行時に読まれていない。フォルダごと退避しても増減ゼロ。没になったVRoid期の残骸で、ディスク22MBのみ |
| `Player.mv1` はテクスチャを内蔵している | **誤り**。`Pura_model_tex.fbm\Pura_basecolor.jpeg` を**相対パスで外部参照**している。他の3体も同様（`Xcode_model_tex.fbm` など） |
| WebView2 が主因 | **誤り**。ゲーム本体プロセス単体で 1,858MB あった |

### 済んだこと

| 施策 | 効果 |
|---|---|
| ミッション演出のフォントサイズを8px刻みに量子化 | **-302MB** |
| ブロックのひびテクスチャを256pxへ縮小 | **-58MB** |

フォントの件は `UIRenderer` が**サイズごとにフォントハンドルを作って二度と消さない**のが原因。
拡縮アニメの途中でサイズを1pxずつ変えると1フレームごとに約4MBのフォントが増え続ける。
**文字サイズをアニメーションさせるときは必ずサイズを丸めること。**

### 残っている削減候補（効果順）

| # | 対象 | 見込み | 内容 |
|---|---|---|---|
| 1 | **キャラ4体を25,000ポリゴンへ** | **-370MB** | `tools/decimate_fbx.py` で実施済み。元FBXは Downloads にある（`PuraFinal/Pura_model_tex.fbx` / `XCodeModel/Xcode_model_tex.fbx` / `BossMac/Mac_model_tex.fbx` / `SafariModel/Safari_lowpoly.fbx`）。**出力FBXの名前は元と同じにすること**（違う名前にすると `.fbm` の参照先が変わって真っ白になる）。mv1化は DxLibModelViewer 手作業 |
| 2 | ブロックの基本テクスチャも256pxへ | -86MB | ひびは実施済み。基本テクスチャは拡張子アイコンを読ませる絵なので、近づいたときの潰れ具合を見てから決める |
| 3 | `Effekseer_Init(8000)` → `4000` / `2000` | -26 / -39MB | 上限を超えたパーティクルは黙って出なくなる。ボス戦の実際の同時数を測ってから決める |
| 4 | 配置物の破片モデルを破壊時に生成する | 未計測 | いまは壊れる前から `<Name>Fractured.mqo` を1個ずつ複製している |
| 5 | `.fbx` ・重複 `.fbm` ・ogg置換済みの mp3 の削除 | ディスク約70MB | メモリには効かない。配布サイズ用 |

縮小には [tools/reduce_stage_textures.py](../tools/reduce_stage_textures.py) が使える
（`--src` で外部PNGを取り込み、`--all` で `assets/model/stage/` を一括縮小。`--size` で解像度を指定）。

---

## 2. 動作確認が済んでいないもの

### 2-1. ギャンブルボックス

`stageBalance.json` ではなく [stageCatalog.json](../assets/data/stageCatalog.json) の
`block_gamble` → `tuning` で調整する。

- `extensionBoostChance` を一時的に `1.0` にすれば必ず当たるので、演出をまとめて確認できる
- **確認が終わったら `0.2` へ戻すこと**
- 期待する挙動
  1. 4回殴ると壊れる
  2. 8割は Xcode が2体、円状に湧く
  3. 2割で画面が紫に光り、中央に `x2` と「拡張子の効果が上がった」が出る
  4. 右下HUDの光の粒が紫・4列・速くなる
  5. インベントリ（E）で能力値と増分が紫、マスの表記が倍の値、
     「現在の能力」の下に `拡張子の効果 x2` のバッジ

### 2-2. RAMブロック

- 5回殴ると壊れ、**取得音**が鳴る
- 右下HUDが「道中で取得」ページへ4秒固定され、下からせり上がる
- 増えた枠は緑の縁。インベントリ（E）でも同じ緑

### 2-3. 敵の攻撃力を上げたぶんの手触り

拡張子でプレイヤーが伸びるぶんを相殺するため、攻撃力だけ上げてある（HPは据え置き）。

| 敵 | Normal | Hard |
|---|---|---|
| Xcode | 55 → 70 | 88 → 112 |
| Safari | 24 → 32 | 41 → 55 |
| Mac | 40 → 55 | 68 → 94 |

**HPを上げてはいけない。** 敵を倒しても報酬が無いため、
HPを増やすと「時間はかかるが得るものはない」戦闘が増えるだけになる。
攻撃力は減算式ダメージの分母側なので、**育ちすぎたビルドにだけ強く効く**という性質がある。

---

## 3. 細かい残り

### 3-1. 検証シーン `DebugDestruction` は削除済み

`START_FROM_DEBUG_SCENE` は `START_FROM_IN_GAME` へ改名した。
BIOSから始めたいときは [DebugFlags.h](../src/core/constant/DebugFlags.h) で `false` にする。

### 3-2. 音が未生成のもの

音源はリポジトリ所有者が作る。実装側で足りていないものは今のところ無い。

---

## 4. やらないと決めたこと（蒸し返さない）

| 内容 | 理由 |
|---|---|
| 実PCの拡張子ヒストグラムからドロップを決める | 企業のPCではフォルダの走査・アップロードがセキュリティ上できないことが多い |
| ZIPブロックの中身を重み付き抽選にする | 開けるまで中身が分からないものにレア度を付けると、拡張子ブロックと役割が重なる。等確率は意図した設計 |
| RAM・ギャンブルボックスを `blockTable` の抽選に入れる | 走破を変える一点物。ランダムに何個も出るとバランスが壊れる。エディタで手置きする |
| ブロックのテクスチャに説明文を入れる | 数メートル離れた3D空間から見るので語は潰れて読めない（`BlockZip.png` の `.zip` だけは例外として残す判断をした） |
| ギャンブルの倍率を重ねがけできるようにする | 2倍で基準値の約3倍になる。3倍まで許すと防御特化ビルドが Mac の通常攻撃まで無効化する |
| 敵のHPを上げてバランスを取る | 上記2-3のとおり |
| prop固有の数値を `stageBalance.json` へ出す | `hitsToBreak` はそのブロックの定義そのもの。外へ出すと2ファイルへidを書くことになり、書き忘れると黙って壊せないブロックが生まれる |

---

## 5. データの置き場（迷ったとき）

| 内容 | ファイル |
|---|---|
| 配置物の素材定義（モデル・絵・大きさ・当たり判定） | `assets/data/stageCatalog.json` の `props[]` |
| 配置物ごとの調整値（打撃回数・ドロップ・確率・倍率） | 同上の各propの `tuning` |
| ブロックの抽選表（グローバルなバランス） | `assets/data/stageBalance.json` |
| 拡張子のボーナス値 | `assets/data/extensionBonus.json` |
| 敵のパラメータ | `assets/data/enemies/*.json`（`gameplay` が Normal、`hard` が上書き） |
| ステージの配置 | `assets/data/stage-test.json`（ゲームが読むのはこれだけ） |

`stageBalance.json` の `type` は `stageCatalog.json` の `props` に存在するidでなければならず、
食い違うと**起動時に例外で止まる**。黙って出現しなくなるのを防ぐため意図的にそうしてある。

---

## 6. 設計上の約束

- **色の意味を画面ごとにずらさない**
  青＝装備中／赤＝道中で変えられない／緑＝増えた枠・上がった値／黄＝強化されている／
  紫＝ギャンブルの当たりで倍率が掛かっている。
  色は [Color.h](../src/core/utility/Color.h) に集約する。Viewへ16進数を直書きしない
- **判定と表示を分ける**
  マスの位置はViewだけが知る（`findSlotIndexAt`）。操作の解釈はシーン、能力の計算はSystem
- **拒否された操作は必ず何かを返す**
  無反応だと「操作が効いていない」と読まれる
- **Viewはイベントを購読しない**
  状態の変化そのものを合図にする（`ExtensionBoostFlashView` は倍率、
  `EquipmentSlotView` は枠数を前フレームと突き合わせている）。
  演出のためだけに配線を増やさない
- **1フレームに複数回 update が回りうる**
  `Application` は処理落ちを取り戻すため最大5回 update する。
  開閉のように「1押しで1回だけ起こしたい」操作は `isKeyPressed` ではなく
  `consumeKeyPress` を使う（前者は同じ1押しに対して update の回数だけ true を返す）
