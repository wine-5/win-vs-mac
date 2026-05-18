## セレクト画面 複数OSWindow方式 実装TODO

### 概要
Windows APIを使ってセレクト画面を複数のOS Windowで実装する。
- 職業選択Window（ListBox）
- ファイル選択Window（3スロット + ListView）
- パラメータ表示Window（GDI描画）
- STAGE SELECTメインWindow（外枠のみ、中身は透明）

各Windowは独立してタスクバーに表示され、EventBus経由でGame層と通信する。
Game層はWindows APIに触れず、インターフェース経由で操作する。

---

### クラス設計一覧

#### Core層（インターフェース・定数・イベント）

| クラス名 | ファイル | 責務 |
|---|---|---|
| `SelectWindowId` | `core/constant/SelectWindowId.h` | Window識別子列挙型（Stage/Job/FileSelect/Parameter） |
| `JobChangedEvent` | `core/event/SelectEvents.h` | 職業選択変更イベント |
| `FileSlotChangedEvent` | `core/event/SelectEvents.h` | ファイルスロット変更イベント |
| `GameStartRequestedEvent` | `core/event/SelectEvents.h` | ゲーム開始要求イベント |
| `IJobProvider` | `core/interface/IJobProvider.h` | 職業データ取得インターフェース |
| `ISelectWindowManager` | `core/interface/ISelectWindowManager.h` | Window管理インターフェース |

#### Game層

| クラス名 | ファイル | 責務 |
|---|---|---|
| `JobDataProvider` | `game/data/JobDataProvider.h/.cpp` | IJobProvider実装（3職業の定数データ保持） |
| `Select`（修正） | `game/scene/Select.h/.cpp` | シーンロジック・EventBus購読・Window管理 |

#### Platform層

| クラス名 | ファイル | 責務 |
|---|---|---|
| `SelectWindowBase` | `platform/select/window/SelectWindowBase.h/.cpp` | Win32Window基底クラス（CreateWindowEx・WndProc・ドラッグ移動） |
| `Win32DrawHelper` | `platform/select/helper/Win32DrawHelper.h/.cpp` | GDI描画ヘルパー（static）（fillRect/drawText/drawProgressBar等） |
| `StageWindow` | `platform/select/window/StageWindow.h/.cpp` | メインWindow外枠（WS_EX_LAYERED で透明） |
| `JobWindow` | `platform/select/window/JobWindow.h/.cpp` | 職業選択Window（ListBox + JobChangedEvent発行） |
| `FileSelectWindow` | `platform/select/window/FileSelectWindow.h/.cpp` | ファイル選択Window（3スロット + ListView + FileSlotChangedEvent発行） |
| `ParameterWindow` | `platform/select/window/ParameterWindow.h/.cpp` | パラメータ表示Window（GDI全描画・EventBus購読） |
| `WindowFactory` | `platform/select/factory/WindowFactory.h/.cpp` | Window生成・破棄の専門クラス |
| `Win32SelectWindowManager` | `platform/select/Win32SelectWindowManager.h/.cpp` | ISelectWindowManager実装（4Window統括管理） |

#### 修正対象

| ファイル | 修正内容 |
|---|---|
| `ServiceLocatorInitializer.h/.cpp` | EventBus / IJobProvider / ISelectWindowManager をServiceLocatorに登録 |

---

### 実装順序（9ステップ）

#### ステップ 1-2：Core層（依存なし）
- [ ] `core/constant/SelectWindowId.h` 作成
- [ ] `core/event/SelectEvents.h` 作成

#### ステップ 2：Core層インターフェース
- [ ] `core/interface/IJobProvider.h` 作成
- [ ] `core/interface/ISelectWindowManager.h` 作成

#### ステップ 3：Game層（職業データ）
- [ ] `game/data/JobDataProvider.h/.cpp` 作成（IJobProvider実装）

#### ステップ 4：Platform層ヘルパー
- [ ] `platform/select/helper/Win32DrawHelper.h/.cpp` 作成

#### ステップ 5：Platform層基底
- [ ] `platform/select/window/SelectWindowBase.h/.cpp` 作成

#### ステップ 6：Platform層Window実装
- [ ] `platform/select/window/StageWindow.h/.cpp` 作成
- [ ] `platform/select/window/JobWindow.h/.cpp` 作成
- [ ] `platform/select/window/FileSelectWindow.h/.cpp` 作成
- [ ] `platform/select/window/ParameterWindow.h/.cpp` 作成

#### ステップ 7：Platform層Factory・Manager
- [ ] `platform/select/factory/WindowFactory.h/.cpp` 作成
- [ ] `platform/select/Win32SelectWindowManager.h/.cpp` 作成

#### ステップ 8：初期化
- [ ] `ServiceLocatorInitializer.cpp` 修正（EventBus / JobDataProvider / Win32SelectWindowManager を登録）

#### ステップ 9：Game層シーン修正
- [ ] `game/scene/Select.h/.cpp` 修正（ISelectWindowManager・EventBus注入、pumpMessages呼び出し）

## ポインタ不要箇所 再評価

### 実装予定
- [ ] `Title::m_fade` を `std::optional` に変更
- [ ] `Select::m_fade` を `std::optional` に変更
- [ ] `PlayerFactory::getPlayer()` に assert を追加（early detection）

---

## 開発が進んだときに行うべきこと

### エフェクトの描画について
- Effekseerのライブラリを使ってDxLibでエフェクトが描画できるかをテストしてみる

### 敵のモデルについて
- 現在の設計では同じモデルを複数生成する分にはメモリ負荷が敵に追加するComponentのみで済むが、別のモデルをたくさん生成するようになるとモデルだけでメモリ負荷がたくさんかかってしまう
- そのため敵のモデルが増えてきたときに、fbxをインポートしてゲームで描画できるような新規ライブラリを追加するべき

### リファクタリング
- Syetem関連のクラスでUpdate内で毎回GetComponentしているため、ISystemクラスに最初にコンポーネントを取得するような純粋仮想関数を追加する -> それを具象クラスですべてのSystemでキャッシュする

- 初期化の際は｛｝で初期化リストで初期化を行うべきだが、代入も{}にしてしまうと可読性が下がることが分かったため、
初期化は｛｝で、代入は = で明示的に書く様にする

- 現在DxLib.hを直接インクルードしているが、ラッパーのヘッダーファイルがあったほうが安全かつ差し替えるときに容易になるため作るようにする

- InGame.cpp内のhファイルが多いためhファイルをインクルードする専用クラスを作成するか検討中

- サービスロケータのみではなくシングルトンのテンプレートクラスの作成して、将来的に差し替えが必要なクラスはサービスロケータにして、インスタンスを1つに制限してテスト等も行う予定がないクラスはシングルトンとして定義してしまったほうがいいと思った

- 一度選択した職業やファイルは記憶しておいて再度挑戦するときにそのまま選択状態にされておくようにする

### デスクトップ画面を作成するときに追加するもの
- ゴミ箱
- セーブする機構を作成して、エクスプローラーに表示してその地点から遊べるように
- 各アプリケーションのアイコンを設置してそこでもWindowをオンオフできるように

- 余裕があったらタスクバーに検索窓をつけて実際に使えるようにしてみたい