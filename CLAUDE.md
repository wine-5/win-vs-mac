# CLAUDE.md

このファイルは Claude AI がこのリポジトリで作業する際に参照するコンテキストです。

---

## プロジェクト概要

「Windows内部を舞台に、自分のPCを武器にしてダンジョンを攻略する」3Dゲーム。
DxLib を使用した Windows 専用アプリケーション。

ターゲット：エンジニア・ギーク層、ゲーム業界の面接官・プログラマー。

詳細: [game_concept.md](docs/design/game_concept.md)

---

## アーキテクチャ

レイヤードアーキテクチャ + Game層内部にECSを採用。

```
platform → infrastructure → game → core
```

- **Core層**: EventBus・ServiceLocator・数学ユーティリティ・インターフェース定義
- **Game層**: ゲームロジック・ECS（Entity/Component/System）・シーン
- **Infrastructure層**: DxLib を使った描画・入力・音声
- **Platform層**: Windows API の処理

**重要**: 内側の層が外側の層をインクルードしてはいけない。DxLib や Windows API はGame層から直接触れない。

詳細: [architecture.md](docs/architecture/architecture.md)

---

## 命名規則

| 対象 | 規則 | 例 |
|---|---|---|
| ファイル名 | PascalCase | `Player.cpp`, `EnemyManager.h` |
| フォルダ名 | 小文字 | `src/`, `game/`, `enemy/` |
| クラス名 | PascalCase | `Player`, `EnemyManager` |
| インターフェース名 | `I` + PascalCase | `ISystemDataProvider` |
| 関数名 | camelCase | `getCpuUsage()`, `updatePlayer()` |
| ローカル変数 | camelCase | `cpuUsage`, `playerPosition` |
| メンバ変数 | `m_` + camelCase | `m_cpuUsage`, `m_playerPosition` |
| 静的メンバ変数 | `s_` + camelCase | `s_instance` |
| 定数 / constexpr（ローカル・グローバル・静的すべて） | UPPER_SNAKE_CASE | `MAX_ENEMY_COUNT`, `SCREEN_WIDTH`, `HORIZONTAL_SHRINK` |
| 列挙型（型名・値） | PascalCase | `EnemyState::Idle` |
| 名前空間 | 小文字 | `namespace game`, `namespace game::player` |

マクロは使用禁止。定数は `constexpr`、インクルードガードは `#pragma once` を使用。

詳細: [naming_convention.md](docs/conventions/naming_convention.md)

---

## コーディング規則

- メンバ変数はクラス定義時に **Uniform Initialization（`{}`）** で初期化する
- **ローカル変数も Uniform Initialization（`{}`）** で初期化する
- **`if` の中身が1行の場合は `{}` を省略する**（複数行のときのみブロックにする）

```cpp
// Good
if (!found)
    return;

// Bad
if (!found)
{
    return;
}
```

- public 関数には必ず **Doxygen コメント**（`@brief`, `@param`, `@return`）を記載する
- スマートポインタは `make_unique` / `make_shared` を使用する

詳細: [naming_convention.md](docs/conventions/naming_convention.md)

---

## コミット規約

Conventional Commits 形式を使用。

```
<type>: <subject>
```

主なtype: `feat`, `fix`, `refactor`, `docs`, `ci`, `chore`, `style`, `perf`, `build`

- subjectは命令形で書く（例: `プレイヤー移動を追加`）
- 末尾にピリオドをつけない
- 50文字以内推奨

### コミットの粒度・運用（Claudeが代行する場合）

- **1コミット＝1つの小さな部品・観点**に絞る。機能をまとめて1コミットにしない。
  - 例: 「攻撃予兆」機能は「扇の描画関数」「予兆Component」「描画System＋配線」「ボスの溜めフェーズ」のように**複数コミットへ分割**する
  - 新規の土台（プリミティブ／Component／データ構造）と、それを使う System・振る舞いは**別コミット**にする
  - リファクタ（命名変更・共通化・不要削除）は**独立コミット**（`refactor:`）にする
- **実装 → 即コミット → 次の実装 → 即コミット** の順で進める（複数機能をまとめて実装後に一括コミットしない。時系列が崩れるため）
- メッセージは**日本語・一行**。`Co-Authored-By` 等のトレーラーや `EOF` は書かない（author はリポジトリ所有者のみ）
- 対象機能のファイルだけを `git add` し、無関係な WIP は含めない
- **push は絶対に行わない**（所有者が行う）

詳細: [commit_convention.md](docs/conventions/commit_convention.md)
