# 次にやること（引き継ぎ）

最終更新: 2026-08-11 / ブランチ `feature/item`

拡張子の「壊す → 拾う → 効く → 付け替える」ループは一通り動く状態まで実装した。
このファイルは、その続きを別の担当が引き取るためのメモ。

---

## 0. 先に読むもの

| ファイル | 内容 |
|---|---|
| [CLAUDE.md](../CLAUDE.md) | アーキテクチャ・命名・コミット規約。**最優先** |
| [docs/architecture/architecture.md](architecture/architecture.md) | レイヤー構成 |
| [docs/conventions/naming_convention.md](conventions/naming_convention.md) | 命名の詳細 |
| [docs/design/extension_swap_design.md](design/extension_swap_design.md) | 拡張子の付け替えの設計 |

### 作業前に必ず守ること

- **ビルド**：`MSBuild.exe DxLib-3D.vcxproj /p:Configuration=Debug /p:Platform=x64`
  （MSBuildは `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\`）
  ゲームが起動したままだと `LNK1168` / `WebView2Loader.dll` のコピー失敗になる。**先に閉じる**
- **コミット**：1コミット＝1つの小さな部品。実装 → 即コミット → 次、の順で進める。
  メッセージは日本語一行の Conventional Commits。`push` は絶対にしない
- **ツールと生成物は別コミット**にする（`tools/gen_*.py` と `assets/` を混ぜない）
- `git add` は自分が触ったファイルだけを明示列挙する。`git add -A` は使わない
  （同じリポジトリで別の担当が並行作業しているため）

### いま未コミットのまま残っているもの（他担当の変更。触らないこと）

- `src/Application.cpp` … 開始シーンを `DebugDestruction` → `InGame` に変えている行
- `src/core/constant/DebugFlags.h`
- `assets/data/stage-test.json` … 差分あり（RAMブロック配置は別担当が編集中の可能性）

---

## 1. 動作確認が済んでいないもの（最初にここから）

実装は入っているが、実機での確認がまだ。**新しい機能を足す前にこれを潰す。**

### 1-1. RAMブロック（装備枠 +1）

- テスト用ステージのプレイヤー正面右（x=220）に1つ置いてある
- 期待する挙動
  1. 5回殴ると壊れる（ひびが3段階で入る）
  2. 右下HUDの「道中で取得」ページが4枠になる（下3つ・上1つ、上段は緑の縁）
  3. 未装備の拡張子を持っていた場合、その先頭が装備中へ繰り上がり**能力値が上がる**
  4. インベントリ（E）でも4枠目が現れる
- 関連：[BlockBreakSystem::grantEquipSlot](../src/game/system/stage/BlockBreakSystem.cpp)、
  [ExtensionEquipSystem::promoteToEquipped](../src/game/system/combat/ExtensionEquipSystem.cpp)

### 1-2. 2段レイアウトの見え方

右下HUDとインベントリの両方で、枠が4つ以上のときに下3つ・上Nの2段になる。
段の中央寄せと、見出しの位置が動かないことを確認する。

### 1-3. リザルトの拡張子表示

クリアかゲームオーバーまで進めて、`ACQUIRED EXTENSIONS` のバッジ列が出るか確認する。
装備中は濃く、所持だけのものは薄く出る。

---

## 2. 隔離フォルダ（未着手・いちばん大きい）

**壊すと敵が出るブロック。ただし低確率で当たりが混ざる。**

### 決まっていること

- ただ敵が出るだけでは壊す動機がないので、**低確率で報酬**を混ぜる
- 案：8割は敵が出る／2割は「ウイルスとして隔離されていた拡張子」が丸ごと手に入る
- 実在のウイルス対策ソフトの隔離フォルダに、誤検知で隔離された無害なファイルが
  入っていることがある、という現実の挙動をそのまま遊びにする

### 決まっていないこと（実装前に相談すること）

- 確率（8:2 が妥当か）
- 当たりのときに何個・どの種別が手に入るか
- 敵が出るときの種類と数
- 壊す前に「危険」と分かるべきか（分かってもなお壊したくなる設計にできるか）

### 実装の見通し

既存の仕組みでほぼ足りる。新しい仕組みを作る前に、下記を使い回せないか検討すること。

1. テクスチャ：`tools/gen_*_texture.py` を1本追加（**文字は入れない**。
   ブロックの面は距離があって語が読めない。記号と色で伝える）
2. モデル・ひび・破片：`tools/gen_stage_models.py` の MANIFEST、
   `tools/gen_crack_textures.py` と `tools/gen_fracture_models.py` の
   `DEFAULT_TARGETS` に名前を足して実行するだけ
3. カタログ：`assets/data/stageCatalog.json` にエントリを足す。
   壊したときの効果は `grantsEquipSlot` と同じ要領で
   `PropDefinition` → `StageProp` → `DestructibleComponent` へ流す
4. 破壊時の処理：`BlockBreakSystem::breakBlock` から分岐。
   敵の出現は `EnemySpawner` が既にある
5. エディタ対応は不要（起動時に `stageCatalog.json` を読んでパレットを作る）

---

## 3. 細かい残り

### 3-1. `BlockZip.png` に文字が残っている

他のブロックからは文字を消したが、これだけ `.zip` の文字が残っている。
`tools/gen_stage_textures.py` 系で作り直すか、手で消す。
消したあとは **ひびテクスチャも作り直すこと**（ひびは元テクスチャに重ねて生成するため、
先に元を直さないと古い絵のまま残る）。

```
python tools/gen_crack_textures.py BlockZip
```

### 3-2. インベントリの「増えた枠」に色が付いていない

右下HUDでは増えた枠を緑の縁（`Color::HUD_BUFF_GREEN`）で示しているが、
インベントリ側は並びだけで色は付けていない。揃えるかどうかは要判断。
揃えるなら [InventoryView::SlotStyle](../src/game/ui/ingame/InventoryView.h) に
旗を1つ足して `drawSlot` の縁の色を分ける。

### 3-3. 単発の16進数色が残っている

HUDの共通2色（面・枠）と固定色は `Color.h` へ集約済み
（`HUD_PANEL_FILL` / `HUD_PANEL_BORDER` / `HUD_LOCKED_RED`）。
以下にはまだ直書きが残っている。

- `src/game/ui/ingame/MiniMapView.cpp`
- `src/game/ui/ingame/PlayerHUDView.cpp`
- `src/game/ui/pause/PauseMenuView.cpp`
- `src/game/ui/debug/DebugHUDView.cpp`（黄色は `Color::YELLOW` で足りる）

### 3-4. 検証シーン `DebugDestruction` の削除

破壊の検証用に作った捨てシーン。本編が安定したら消す。
参照箇所：`SceneFactory.cpp` / `SceneFactory.h` / `SceneType.h` / `DebugDestruction.*`、
`DebugFlags.h` の `START_FROM_DEBUG_SCENE`、`Application.cpp` の開始シーン分岐。

### 3-5. 音が未生成のもの

音源はリポジトリ所有者が作る。実装側で足りていないものは今のところ無い。
新しく必要になったら [locked_slot_sound_spec.md](design/locked_slot_sound_spec.md) と
同じ形式で仕様を書いてから依頼する。

---

## 4. やらないと決めたこと（蒸し返さない）

| 内容 | 理由 |
|---|---|
| 実PCの拡張子ヒストグラムからドロップを決める | 企業のPCではフォルダの走査・アップロードがセキュリティ上できないことが多い |
| ZIPブロックの中身を重み付き抽選にする | 開けるまで中身が分からないものにレア度を付けると、拡張子ブロックと役割が重なる。等確率は意図した設計 |
| RAMブロックを `blockTable` の抽選に入れる | 枠が増える一点物。ランダムに何個も出るとバランスが壊れる。エディタで手置きする |
| ブロックのテクスチャに説明文を入れる | 数メートル離れた3D空間から見るので語は潰れて読めない。記号・色・数字で伝える |

---

## 5. 今回入れたものの要点（引き継ぎ用の地図）

| 機能 | 主なファイル |
|---|---|
| ブロック破壊・ひび・破片 | `system/stage/BlockBreakSystem`、`BlockDebrisSystem` |
| 欠片のドロップと取得 | `system/stage/ExtensionPickupSystem` |
| 能力への反映・入れ替え | `system/combat/ExtensionEquipSystem` |
| 持ち物 | `component/combat/ExtensionInventoryComponent`（`m_maxEquipped` は可変） |
| インベントリ（E） | `ui/ingame/InventoryView` |
| 右下スロット | `ui/ingame/EquipmentSlotView` |
| 付け替え端末（F2） | `system/stage/RenameTerminalSystem`、`ui/ingame/InteractPromptView` |
| 光の粒（共通部品） | `ui/ingame/OrbitGlow` |
| 能力値の収集（共通） | `game/utility/PlayerStats` |

### 設計上の約束

- **色の意味を画面ごとにずらさない**
  青＝装備中／赤＝道中で変えられない／緑＝増えた枠・上がった値／黄＝強化されている
- **判定と表示を分ける**
  マスの位置はViewだけが知る（`findSlotIndexAt`）。操作の解釈はシーン、
  能力の計算はSystem
- **拒否された操作は必ず何かを返す**
  無反応だと「操作が効いていない」と読まれる。固定枠へ落とすと震えて音が鳴る
- **入れ替えは即時反映**
  インベントリを開いている間は時間が止まりSystemのupdateが回らないため、
  付け替えだけはイベントの購読側で同期的に処理している
