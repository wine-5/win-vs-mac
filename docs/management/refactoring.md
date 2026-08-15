# リファクタリング診断・分離計画

対象: src 全体（403ファイル / 約40,000行）
目的: 責務違反の検出と、肥大化クラス（InGame 1163行 等）の分離方針の決定

## 全体所見

レイヤー依存（core → game → infrastructure）は一方向に保たれており、System・Factory・View の分割粒度も適切。前回リファクタで指摘した箇所の多くは解消済み。残る問題は以下の2点に絞られる。

---

## 問題1: InGame（.cpp 1163行 + .h 320行）— 4つの責務が混在

### 責務の分解

| 責務 | 該当コード | 行数感 |
|---|---|---|
| ① シーン構築（環境設定・View群の生成・System登録） | コンストラクタ後半 + `setupSystems()` | 約550行 |
| ② ゲーム進行ルール（雑魚全滅→ボス出現→勝敗判定→リザルト保存） | `setupEvents()`, `spawnBoss()`, `killRemainingEnemies()`, `saveResultData()`, `m_stageEnemyIds`, `m_elapsedTime` 等 | 約250行 |
| ③ インベントリ/付け替えの入力コントローラ | `updateInventory()`, `updateRenameTerminal()`, `updateSwapSelection()`, `requestSwap()`, `setInventoryOpen()` | 約200行 |
| ④ シーンのライフサイクル（update/drawの委譲、ポーズ連動、ヒットストップ） | `update()`, `draw()`, `onPauseChanged()` | 約100行 |

④だけがシーンクラス本来の仕事。①②③を切り出す。

### 分離① InGameSetup — 最優先・効果最大

`setupSystems()` の350行（System登録の羅列 + 順序コメント）、コンストラクタ内のView生成150行、ライティング/フォグ設定、無名名前空間の `buildTabProjectileSetup` / `buildRainbowSetup` は、すべて構築時のみの処理でシーンの実行時状態と無関係。

```
game/scene/ingame/
  InGameSetup.h / .cpp     ← System登録・View生成・環境設定を担う
```

Setupの生成物は構造体で返す。

```cpp
// InGameSetup が構築して返すもの
struct InGameContext
{
    // Viewが描画フェーズで参照するSystemポインタ群
    system::visual::BattleStartSystem* m_battleStartSystem{};
    system::stage::RenameTerminalSystem* m_renameTerminalSystem{};
    // View群のunique_ptr
    std::unique_ptr<ui::ingame::PlayerHUDView> m_playerHUDView;
    // ...
};
```

これで InGame.cpp から約500行が消え、InGame は「進行」だけのクラスになる。

注意: System登録の順序コメント（「押し返しの前に」「AttackSystemの後に」等）は貴重な仕様ドキュメントなので、そのまま Setup 側へ移すこと。

### 分離② MissionProgress — ゲーム進行ルール

「雑魚IDを追跡→全滅でボス出現→ボス撃破で残敵一掃→リザルト保存」は勝敗仕様そのもの。EventBus 購読で完結しているため綺麗に切れる。

```cpp
// game/scene/ingame/MissionProgress.h
class MissionProgress
{
public:
    MissionProgress(core::base::EventBus& eventBus,
        core::ecs::ComponentManager& componentManager,
        factory::EnemySpawner& enemySpawner,
        core::iface::IResourceManager& resourceManager,
        GameManager& gameManager);

    void registerInitialEnemies();          // spawnEntities末尾の処理
    void update(float scaledDeltaTime);     // 経過時間・計測制御
    [[nodiscard]] int remainingEnemyCount() const noexcept;
    [[nodiscard]] core::ecs::EntityId bossId() const noexcept;
    [[nodiscard]] float elapsedTime() const noexcept;
private:
    void spawnBoss();
    void killRemainingEnemies(core::ecs::EntityId excludedId) noexcept;
    void saveResultData(bool isVictory) noexcept;
    // m_stageEnemyIds, m_macId, m_killCount, m_totalDamageTaken,
    // m_elapsedTime, m_isTimeMeasuring, m_subscriptions をここへ移動
};
```

- `draw()` で渡している `m_stageEnemyIds.size()` / `m_macId` / `m_elapsedTime` はすべてゲッターに置き換わる
- ヒットストップ（`m_hitStop`）はクリティカルイベント購読とセットなので同居させてよい

### 分離③ InventoryController

`updateSwapSelection()` 一帯は「マウス入力を解釈して View に問い合わせ、イベントを発行する」典型的なコントローラ。InGame 本体とは PauseManager の共有以外に接点がない。

```cpp
// game/ui/ingame/InventoryController.h
class InventoryController
{
public:
    void update();  // E/F2/Esc/マウスの全処理
    [[nodiscard]] bool isOpen() const noexcept;
private:
    // m_isSwapMode, m_swapHeldIndex, m_wasMouseLeftDown をここへ
};
```

置き場所は `ui/ingame/`（InventoryView の隣）。View（描画）と Controller（入力解釈）が対になり追いやすくなる。

### 分離後の InGame（想定150〜200行）

```cpp
InGame::InGame(...) : ...
{
    loadResources();
    spawnEntities();
    InGameSetup setup{ ... };
    m_context = setup.build(m_view);        // ← System登録・View生成・環境設定
    m_missionProgress.registerInitialEnemies();
}

void InGame::update(float deltaTime)
{
    m_inventoryController.update();
    if (m_pauseManager.isPausedBy(PauseReason::Inventory)) return;

    const float scaled{ m_hitStop.apply(deltaTime) };
    m_missionProgress.update(scaled);
    m_systemManager.update(scaled);
    m_view.setInteractTarget(...);
}

void InGame::draw()
{
    m_view.draw(m_playerId, m_missionProgress.remainingEnemyCount(),
        m_missionProgress.bossId(), m_missionProgress.elapsedTime());
}
```

---

## 問題2: InGame↔View 間の「set○○System」パターン — 配線コストの肥大化

InGameView の setter は26個あり、InGame.cpp の至る所に以下のペアが散っている。

```cpp
auto* xxx = m_systemManager.registerSystem<XxxSystem>(...);
m_view.setXxxSystem(xxx);
```

「描画順は View が決め、描画内容は System が持つ」という設計判断自体は正しい。問題は配線コスト。

### 軽い案（推奨）

setter を全部やめて、`InGameContext`（分離①の生成物）を丸ごと View に渡す。

```cpp
m_view.attach(context);  // setterの26呼び出しが1回に
```

### 重い案（今はやらない）

「View から描かれる System」に `IDrawableSystem` インターフェースを導入し、描画レイヤー（3D背景 / 3D前景 / HUD奥 / HUD手前）を enum で宣言させ、View はレイヤー順に回すだけにする。描画順コメント（「HPバーより手前」等）という仕様が型に落ちる利点はあるが、終盤に行う変更量ではない。

---

## 分割不要と判断したもの

| クラス | 行数 | 判断理由 |
|---|---|---|
| InventoryView | 1065 | 責務は「インベントリ画面の描画」一本。private描画ヘルパーに分割済みで、状態も描画演出用のみ。エクスプローラ風UIの見た目が複雑なだけ。分割すると座標計算の共有が逆に面倒になる |
| ModelRepository | 641 | 半分近くが json パース。気になるなら `ModelMetadataParser` として切れるが、Repository＝「読み込みと保持」の責務内。終盤の今は触らない |
| BattleStartSystem | 493 | 演出の仕様が大きいだけで責務は単一 |
| MacAISystem | 488 | ボスFSMの仕様が大きいだけで責務は単一 |
| EnemyData / PlayerData | 339 / 289 | ゲッターの羅列で行数が出ているだけ。健全 |
| GameManager | - | 「シーン間で持ち回る共有データ」に収まっており問題なし |
| ServiceLocator | - | 多数箇所から使われるもの（IAudioManager 33箇所、IUIRenderer 27箇所）が中心で方針どおり。ICamera/IRenderer/IAnimator がロケータ登録なのに InGame へはコンストラクタ注入という二重経路だけ若干不揃いだが実害なし |

---

## 推奨作業順

1. **InGameSetup 切り出し**（InGame -500行。移動のみでロジック変更なし。setter は Context 一括渡しに置換）
2. **MissionProgress 切り出し**（-250行。イベント購読の移動）
3. **InventoryController 切り出し**（-200行）

いずれも「コードの移動」であって書き換えではないため、終盤でも安全に実施できる。

### 実施時の注意

- **EventBus の破棄順**: InGame.h 先頭のコメント（購読者より前に EventBus を宣言）の制約は、MissionProgress 切り出し後も同じく残る。MissionProgress のメンバ宣言は EventBus より後（＝破棄が先）に置くこと
- System 登録の順序コメントは仕様ドキュメントとして Setup 側へ必ず引き継ぐこと


# コードレビュー：検出された問題点

対象：`src/`（約 40,000 行 / 403 ファイル）
観点：ポインタ・所有権・パフォーマンス

---

## 総評

`new` / `delete` の直接使用はゼロ。所有権は `unique_ptr`、非所有は生ポインタ・参照で一貫して表現されており、設計は良好。以下は「さらに良くする」ための指摘。

| # | 問題 | 深刻度 | 修正コスト |
|---|---|---|---|
| 1 | `ServiceLocator::provide` のポインタ調整漏れ | **高**（潜在） | 1 行 |
| 2 | `getAllEntities()` の毎フレームアロケーション | 中 | 中 |
| 3 | `has()` → `get()` の二重ハッシュ検索 | 低〜中 | 小（機械的） |
| 4 | `ComponentArray::add()` の余分なコピー | 低 | 小 |

---

## 1. `ServiceLocator::provide` のポインタ調整漏れ

**場所**：`core/base/ServiceLocator.h`
**深刻度**：高（現状は未発症。多重継承を導入した瞬間に未定義動作）

### 現状

```cpp
template<typename TInterface, typename TImpl>
static void provide(std::unique_ptr<TImpl> service)
{
    registerService(std::type_index(typeid(TInterface)),
                    std::shared_ptr<void>(std::move(service)));  // TImpl* のまま消える
}
```

呼び出し側は `provide<IStringConverter>(std::make_unique<StringConverter>())` の形。
オーバーロード解決の結果、**全 17 箇所がこの 2 引数版を通る**（`TImpl` 側が完全一致で勝つ）。

このため `shared_ptr<void>` に入るのは `TImpl*`。一方 `get<T>()` は：

```cpp
return static_cast<T*>(it->second.get());   // void* を TInterface* とみなす
```

`TImpl*` → `TInterface*` の変換に必要な**ポインタ調整が行われない**。

### 実測（多重継承の場合）

```
AudioManager* のアドレス   : 0x558bdbda62b0
IAudioManager* へ正しく変換: 0x558bdbda62b8  <- 8 バイトずれる
void* 経由で復元           : 0x558bdbda62b0  <- ずれない = 誤り
一致するか: いいえ（未定義動作）
```

ずれたアドレスで仮想関数を呼ぶと、別インターフェースの vtable を引く。
単一継承では基底がオフセット 0 に来るため、**現状は偶然動いている**。

### 修正

```cpp
template<typename TInterface, typename TImpl>
static void provide(std::unique_ptr<TImpl> service)
{
    // TInterface へ変換してから型を消す（ここで調整が入る）
    std::unique_ptr<TInterface> asInterface{ std::move(service) };
    registerService(std::type_index(typeid(TInterface)),
                    std::shared_ptr<void>(std::move(asInterface)));
}
```

---

## 2. `getAllEntities()` の毎フレームアロケーション

**場所**：`core/ecs/ComponentArray.h` / `ComponentManager.h`、呼び出し **56 箇所**
**深刻度**：中

### 現状

```cpp
std::vector<EntityId> getAllEntities() const   // 値返し = 毎回ヒープ確保
{
    std::vector<EntityId> entities;
    entities.reserve(m_component.size());
    for (const auto& [id, _] : m_component) entities.push_back(id);
    return entities;
}
```

呼び出し側の典型：

```cpp
const auto entities{ m_componentManager.getAllEntities<ColliderComponent>() };
for (const auto id : entities)
{
    const auto& collider{ m_componentManager.get<ColliderComponent>(id) };  // 再検索
}
```

**確保 → 詰める → ID で引き直す**の 3 段階。ID からの再検索はハッシュ検索。

呼び出しが集中しているファイル：

| ファイル | 箇所数 |
|---|---|
| `DebugGizmoView.cpp` | 4 |
| `InGameView.cpp` | 4 |
| `MiniMapView.cpp` | 3 |
| `DetectionAlertVisualsSystem.cpp` | 3 |
| （他 20 ファイル） | 42 |

### 修正案：`forEach` を追加する

```cpp
// ComponentArray
template <typename Fn>
void forEach(Fn&& fn)
{
    for (auto& [id, comp] : m_component) fn(id, comp);
}

// ComponentManager
template <typename T, typename Fn>
void forEach(Fn&& fn) { getComponentArray<T>()->forEach(std::forward<Fn>(fn)); }
```

```cpp
m_componentManager.forEach<ColliderComponent>(
    [&](EntityId id, ColliderComponent& collider) { ... });
```

**アロケーションとハッシュ再検索の両方が消える。**

### 補足

`ComponentArray` のコメントに「利用側を変えずに packed array へ差し替えられる」とあるが、
`getAllEntities` が残っていると「ID を受け取って引き直す」形が固定されてしまう。
`forEach` を経由させておくと、その差し替えが本当に無痛になる。

---

## 3. `has()` → `get()` の二重ハッシュ検索

**場所**：`has<` の呼び出しが **133 箇所**。うち直後に `get<` が続くものが対象
**深刻度**：低〜中（AI 系は毎フレーム全敵を走査するため効く）

### 現状

```cpp
if (!m_componentManager.has<AIComponent>(entityId))    // find #1
    continue;
auto& ai{ m_componentManager.get<AIComponent>(entityId) };  // find #2
```

`has()` も `get()` も内部は `m_component.find(id)`。**同じキーを 2 回引いている。**

### 修正

```cpp
auto* ai{ m_componentManager.tryGet<AIComponent>(entityId) };
if (!ai) continue;
```

または C++17 の初期化付き `if`：

```cpp
if (auto* ai{ m_componentManager.tryGet<AIComponent>(entityId) })
{
    // このブロック内で ai は非 nullptr が保証される
}
```

### 優先して直す箇所

| ファイル:行 | 備考 |
|---|---|
| `system/ai/MeleeChaseAISystem.cpp:45, 101, 115` | 毎フレーム全敵 |
| `system/ai/EnemyRangedAttackSystem.cpp:46` | 同上 |
| `system/ai/DetectionSystem.cpp:30-34` | **2 組あり 4 回 → 2 回** |

`tryGet` は既に 93 箇所で使われているため、方針は統一済み。取りこぼしの回収にあたる。

---

## 4. `ComponentArray::add()` の余分なコピー

**場所**：`core/ecs/ComponentArray.h` / `ComponentManager.h`
**深刻度**：低

### 現状

```cpp
void add(EntityId id, T component)   // コピー #1（値渡し）
{
    m_component[id] = component;     // コピー #2（代入）
}
```

`ComponentManager::add()` も同様に値で受けて値で渡している。
Component が小さいうちは誤差だが、`std::string` や `std::vector` を含む型では効く。

### 修正

```cpp
// ComponentArray
template <typename U = T>
void add(EntityId id, U&& component)
{
    m_component.insert_or_assign(id, std::forward<U>(component));
}

// ComponentManager
template <typename T, typename U = T>
void add(EntityId id, U&& component)
{
    getComponentArray<T>()->add(id, std::forward<U>(component));
}
```

---

## 良い点（維持すべき設計）

- `new` / `delete` の直接使用なし（`unique_ptr` 88 / `shared_ptr` 8）
- 所有＝`unique_ptr`・値、非所有＝`T*`・`T&` の使い分けが全体で一貫
- `ObjectPool` の `m_all`（`unique_ptr`）と `m_available`（`T*`）の分離
- `vector<unique_ptr<T>>` によるポインタ安定性の確保（`expand()` で借用ポインタが無効化しない）
- `EventBus::Subscription` による RAII での購読解除
- `InGame` のメンバ宣言順コメント（破棄順の明示）
- `ServiceLocator::clear()` の `m_order` による逆順破棄
- `get()` は assert、`tryGet()` は `nullptr` という意図の撃ち分け
- `[[nodiscard]]` の付与

---

## 推奨する着手順

1. **問題 1**（1 行。将来の地雷除去）
2. **問題 3**（機械的。AI 系のみ先行でも可）
3. **問題 4**（`add` 周りのみ）
4. **問題 2**（`forEach` 追加 → 呼び出し 56 箇所を段階的に移行）