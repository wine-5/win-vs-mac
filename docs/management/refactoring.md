# 残りの修正項目


### 6. エンティティIDのキャッシング（パフォーマンス改善） 🟡 **重要**

#### 現状の問題
`InGameScene::update()` で、毎フレーム同じエンティティIDを取得：
```cpp
// 毎フレーム実行（60FPS = 1秒間に60回）
auto& transform = m_componentManager.get<TransformComponent>(m_objectFactory.getPlayer().getId());
auto& render = m_componentManager.get<RenderComponent>(m_objectFactory.getPlayer().getId());
auto& anim = m_componentManager.get<AnimationComponent>(m_objectFactory.getPlayer().getId());
// ... 6回以上の重複呼び出し
```

#### 改善方針
コンストラクタまたは初期化時にキャッシュ：
```cpp
// InGameScene.h (private:)
core::ecs::EntityId m_playerId{};
core::ecs::EntityId m_groundId{};

// InGameScene.cpp (コンストラクタまたはspawnEntities()内)
m_playerId = m_objectFactory.getPlayer().getId();
m_groundId = m_ground->getId();

// update()内
auto& transform = m_componentManager.get<TransformComponent>(m_playerId);
auto& render = m_componentManager.get<RenderComponent>(m_playerId);
```

---

### 7. const correctness と noexcept の追加 🟢 **推奨**

#### 対象
例外を投げないgetter関数に `noexcept` を追加：

```cpp
// Before
core::ecs::EntityId getId() const { return m_entity.getId(); }

// After
core::ecs::EntityId getId() const noexcept { return m_entity.getId(); }
```

#### 対象ファイル
- `src/game/actor/Player.h`
- `src/game/stage/Ground.h`
- `src/core/ecs/Entity.h`
- `src/game/data/PlayerData.h` - すべてのgetter
- `src/game/data/GroundData.h` - すべてのgetter

#### メリット
- コンパイラの最適化が可能になる
- 意図が明確になる（この関数は絶対に例外を投げない）

---

### 8. エラーハンドリングの強化 🟡 **重要**

#### 現状の問題
`assert()` はReleaseビルドで無効化される：
```cpp
assert(playerMeta.has_value() && "Playerのメタデータが見つかりません");
```

#### 改善方針
本番環境でも動作するエラーハンドリングを追加：
```cpp
if (!playerMeta.has_value()) {
    LOG("ERROR", "Playerのメタデータが見つかりません");
    throw std::runtime_error("Failed to load player metadata");
}
```

#### 対象箇所
- `src/game/scene/InGameScene.cpp` - メタデータ取得時
- `src/infrastructure/ResourceManager.cpp` - JSON読み込み失敗時
- `src/game/data/PlayerData.h` - `fromMetadata()` 内

#### 注意点
- 開発中は `assert()` も残す（デバッグに有用）
- 本番用エラーハンドリングを追加で実装

---

### 9. ResourceManager の責務分離（将来的な改善） 🔵 **低優先度**

#### 現状
`ResourceManager` が複数の責務を持つ：
- JSONパース
- モデルロード
- キャッシュ管理

#### 将来的な分離案
```
infrastructure/
├─ ResourceManager.h / .cpp      // ファサード
├─ JsonLoader.h / .cpp           // JSON読み込み専用
└─ ModelCache.h / .cpp           // キャッシュ管理専用
```

#### 注意
- **EditorTool実装前には実施しない**
- PR_feature_collider.md の「技術的負債の記録」に既に記載済み
- EditorTool実装後、必要に応じて実施

---

### 10. 依存関係の逆転（Clean Architecture強化） 🔵 **低優先度**

#### 現状の問題
`InGameScene.cpp` が具体的なSystem実装に直接依存：
```cpp
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
// ... 増える一方
```

#### 改善案
`InGameSceneInitializer` などに委譲：
```cpp
// infrastructure/InGameSceneInitializer.h
class InGameSceneInitializer {
public:
    static void setupSystems(SystemManager&, ...);
};
```

#### 注意
- **EditorTool実装前には実施しない**
- システムが増えてから検討

---

## 実装順序（推奨）

### フェーズ1：ドキュメント整備（1-2日） 🔴
- [x] 1. public関数へのDoxygenコメント追加

### フェーズ2：InGameScene リファクタリング（1日） 🔴
- [x] 4. コンストラクタの責務分離 (`loadResources()`, `spawnEntities()`, `setupSystems()`)
- [x] 6. エンティティIDのキャッシング

### フェーズ3：Factory Pattern導入（2-3日） 🟡
- [x] 2. IFactory + PlayerFactory + GroundFactory + FactoryManager 実装

### フェーズ4：細かい改善（1日） 🟢
- [x] 3. メンバ変数の初期化統一
- [x] 5. 命名規則の修正 (`m_system` → `m_systems`)
- [x] 7. const/noexcept の追加
- [x] 8. エラーハンドリングの強化

### フェーズ5：EditorTool開始 ✨
→ リファクタリング完了後、`feature/editor-scene` ブランチ作成

### フェーズ6：将来的な改善（EditorTool実装後） 🔵
- [ ] 9. ResourceManager の責務分離
- [ ] 10. 依存関係の逆転

---

## 完了条件

以下をすべて満たしたら EditorTool 実装に移行：
- ✅ すべてのpublic関数にDoxygenコメント
- ✅ InGameScene コンストラクタが50行以内
- ✅ Factory Pattern が導入され、ObjectFactory が削除されている
- ✅ メンバ変数の初期化が統一されている
- ✅ コンパイル・実行が正常に動作
- ✅ 既存機能（Player移動、コライダー可視化、衝突判定）が正常動作

---

## 備考

### Git管理
- ブランチ名: `refactor/clean-architecture`
- コミットメッセージ例:
  - `docs: public関数にDoxygenコメント追加`
  - `refactor: InGameSceneコンストラクタを3メソッドに分割`
  - `feat: Factory Patternを導入（IFactory + PlayerFactory + FactoryManager）`

### テスト
各フェーズ完了後、以下を確認：
- [x] コンパイルエラーがない
- [x] Player移動が正常動作
- [x] コライダー可視化が正常動作
- [x] 衝突判定が正常動作
- [x] カメラ追従が正常動作

### 就活作品としてのアピールポイント
- **ドキュメント**：すべての関数にコメントがあり、他者が見ても理解しやすい
- **設計パターン**：Factory Pattern を適切に導入し、拡張性を確保
- **Clean Architecture**：責務分離、依存関係逆転を意識した設計
- **保守性**：統一された命名規則、初期化方法、エラーハンドリング
