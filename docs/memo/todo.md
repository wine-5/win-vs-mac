## ファイル拡張子スロット機能 実装TODO

### 概要
セレクト画面でWindowsのファイルを選択し、拡張子に応じてPlayerのパラメータを補正する。
シーン間のデータ受け渡しは `FileEquipmentData`（Bridge役）で行う。

---

### クラス設計一覧

| クラス名 | 層 | ファイル | 責務 |
|---|---|---|---|
| `IFileSelector` | Core/interface | `core/interface/IFileSelector.h` | ファイル選択処理の抽象インターフェース |
| `WindowsFileSelector` | Platform | `platform/WindowsFileSelector.h/.cpp` | `GetOpenFileName` を使ったファイル選択の実装 |
| `FileEquipmentData` | Game/data | `game/data/FileEquipmentData.h` | 選択ファイルのパス・hasSelection を保持するデータ構造体。`GameManager` が唯一の所有者 |
| `ExtensionBonus` | Game/data | `game/data/ExtensionBonus.h` | 拡張子ボーナス値（atk/spd/def/hp/range）の構造体 |
| `ExtensionBonusCalculator` | Game/utility | `game/utility/ExtensionBonusCalculator.h` | 拡張子文字列 → `ExtensionBonus` を算出するロジック |
| `GameManager` | Game | `game/GameManager.h/.cpp` | ゲーム全体の状態管理。`FileEquipmentData` を所有し、ServiceLocator に登録される |

---

### 拡張子ボーナス仕様

| 拡張子グループ | 補正パラメータ |
|---|---|
| .exe .dll .bat | ATK+ |
| .txt .pdf .docx | SPD (moveSpeed)+ |
| .png .jpg .bmp | DEF+ |
| .mp3 .wav .flac | MaxHP+ |
| .zip .7z .rar | 全パラメータ小+ |
| それ以外 | attackRange+ |

---

### 実装フェーズ

---

#### フェーズ1：ファイル1つ選択・パラメータ補正

**新規作成**
- [ ] `core/interface/IFileSelector.h` ― ファイル選択IF（`selectFile()` → `std::string` を返す）
- [ ] `platform/WindowsFileSelector.h/.cpp` ― GetOpenFileName 実装（フィルタ：すべてのファイル）
- [ ] `game/data/FileEquipmentData.h` ― `selectedFilePath` / `hasSelection` の1ファイル構造体
- [ ] `game/data/ExtensionBonus.h` ― ボーナス値構造体（atk/spd/def/hp/range）
- [ ] `game/utility/ExtensionBonusCalculator.h` ― 拡張子文字列 → `ExtensionBonus` を算出
- [ ] `game/GameManager.h/.cpp` ― `FileEquipmentData` 所有・ServiceLocator 登録

**既存ファイル変更**
- [ ] `ServiceLocatorInitializer.cpp` ― GameManager・WindowsFileSelector を登録
- [ ] `SceneFactory.cpp` ― GameManager から FileEquipmentData 参照を取得し StageSelect・InGame へ注入
- [ ] `StageSelect.h/.cpp` ― ファイル選択ボタン追加・IFileSelector 依存追加・FileEquipmentData 書き込み
- [ ] `InGame.h/.cpp` ― FileEquipmentData 参照受け取り・ExtensionBonus を計算して Player パラメータ補正

---

#### フェーズ2：ファイル最大5つ選択・パラメータ合算（フェーズ1完了後）

**変更方針**
- `FileEquipmentData` の `selectedFilePath` を `std::array<std::string, MAX_SLOT>` に変更
- `hasSelection` を `std::array<bool, MAX_SLOT>` に変更（または有効スロット数で管理）
- UI に「スロット1〜5」のファイル選択ボタンを追加
- InGame 側で全スロットの `ExtensionBonus` を合算して Player に適用

**変更が必要なファイル**
- [ ] `game/data/FileEquipmentData.h` ― `MAX_SLOT = 5` のスロット配列へ変更
- [ ] `StageSelect.h/.cpp` ― スロットボタンを5個に拡張
- [ ] `InGame.h/.cpp` ― 全スロット分のボーナスを合算する処理に変更

---

### データフロー（フェーズ1）

```
[StageSelect] ファイル選択ボタン押下
     ↓ IFileSelector::selectFile()
[WindowsFileSelector] GetOpenFileName → ファイルパスを返す
     ↓
[StageSelect] FileEquipmentData に selectedFilePath / hasSelection を書き込み
     ↓（シーン遷移）
[InGame] FileEquipmentData を読み取り
     ↓ ExtensionBonusCalculator::calculate(extension)
[ExtensionBonus] atk/spd/def/hp/range のボーナス値
     ↓
[Player] 基礎パラメータにボーナスを加算して生成
```

## 開発が進んだときに行うべきこと

### 敵のモデルについて
- 現在の設計では同じモデルを複数生成する分にはメモリ負荷が敵に追加するComponentのみで済むが、別のモデルをたくさん生成するようになるとモデルだけでメモリ負荷がたくさんかかってしまう
- そのため敵のモデルが増えてきたときに、fbxをインポートしてゲームで描画できるような新規ライブラリを追加するべき

### リファクタリング
- Syetem関連のクラスでUpdate内で毎回GetComponentしているため、ISystemクラスに最初にコンポーネントを取得するような純粋仮想関数を追加する -> それを具象クラスですべてのSystemでキャッシュする

- 初期化の際は｛｝で初期化リストで初期化を行うべきだが、代入も{}にしてしまうと可読性が下がることが分かったため、
初期化は｛｝で、代入は = で明示的に書く様にする

- 現在DxLib.hを直接インクルードしているが、ラッパーのヘッダーファイルがあったほうが安全かつ差し替えるときに容易になるため作るようにする

- InGame.cpp内のhファイルが多いためhファイルをインクルードする専用クラスを作成するか検討中