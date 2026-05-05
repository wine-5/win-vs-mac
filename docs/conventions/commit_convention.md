# コミットメッセージ規約

本プロジェクトでは **Conventional Commits** の形式に従ったコミットメッセージを使用します。

## 基本フォーマット

```
<type>: <subject>

[optional body]

[optional footer]
```

### 例
```
feat: プレイヤー移動機能を追加

WASDキーでプレイヤーを移動できるようにした。
MoveSystemを実装。
```

---

## Type（種類）一覧

| Type | 説明 | 使用例 |
|------|------|--------|
| **feat** | 新機能追加 | `feat: プレイヤー移動を追加` |
| **fix** | バグ修正 | `fix: 衝突判定のバグを修正` |
| **refactor** | リファクタリング | `refactor: PlayerクラスをECS対応に変更` |
| **docs** | ドキュメント変更 | `docs: 命名規則のmdを更新` |
| **ci** | CI/CD設定変更 | `ci: GitHub Actionsを追加` |
| **chore** | 雑務・設定変更 | `chore: .gitignoreを更新` |
| **style** | コードスタイル修正 | `style: インデントを統一` |
| **perf** | パフォーマンス改善 | `perf: 描画処理を最適化` |
| **test** | テスト追加・修正 | `test: CollisionSystemのテストを追加` |
| **build** | ビルド関連の変更 | `build: vcxprojにファイルを追加` |
| **revert** | コミットの取り消し | `revert: "feat: 実験的機能を追加"` |

---

## Subject（件名）のルール

1. **命令形で書く**
   - ✅ `プレイヤー移動を追加`
   - ❌ `プレイヤー移動を追加した` / `プレイヤー移動を追加します`

2. **クラス名・ファイル名は命名規則に従う**
   - ✅ `feat: PlayerクラスにHP管理機能を追加`
   - ✅ `fix: MoveSystem.cppのバグを修正`
   - ❌ `feat: playerクラスにHP管理機能を追加`（クラス名は`Player`）

3. **末尾にピリオド（。）をつけない**
   - ✅ `feat: カメラ追従機能を追加`
   - ❌ `feat: カメラ追従機能を追加。`

4. **50文字以内に収める**（推奨）

---

## スコープ（オプション）

変更範囲を明示したい場合は括弧で囲む。

```
feat(player): ジャンプ機能を追加
fix(collision): AABB判定の誤差を修正
refactor(ecs): ComponentArrayをテンプレート化
```

---

## 参考資料

- [Conventional Commits](https://www.conventionalcommits.org/)
- [Angular Commit Message Guidelines](https://github.com/angular/angular/blob/main/CONTRIBUTING.md#commit)
