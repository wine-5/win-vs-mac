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