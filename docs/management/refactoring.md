# リファクタリング診断・分離計画

対象: src 全体（403ファイル / 約47,000行）
最終更新: 2026-08-20（ゲームパッド対応が一段落した時点で全面的に再診断）
目的: 責務違反の検出と、肥大化クラスの分離方針の決定

---

## 全体所見

レイヤー依存（core → game → infrastructure → platform）は一方向に保たれており、System・Factory・View の分割粒度も概ね適切。`new` / `delete` の直接使用はゼロで、所有権表現も一貫している。

前回診断からの変化は以下の2点。

- **InventoryView が 1065 → 1263行に増え、リポジトリ最大のファイルになった**。前回「分割不要」と判断したが、その判断はもう成立しない（後述）
- **InGame は分離が未着手のまま 1163 → 1259行**。ゲームパッド対応で `updateInput()` が増えた
- InGameView の setter は 26 → 30個。配線コストの問題は放置すると増え続けることが実証された

分離すべきクラスは **InGame・InventoryView・InGameView・InputManager の4件**。Win32SelectWindowManager は責務が2つあるが、優先度の判断として今は触らない。

---

## 分離対象のまとめ（優先度順）

| # | クラス | 現在 | 分離するもの | 削減見込み | 難度 |
|---|---|---|---|---|---|
| 1 | InGame | 1259行 | InGameSetup / MissionProgress / InventoryController | -947行 | 低（移動のみ） |
| 2 | InventoryView | 1263行 | InventoryStatsPane | -200行 | 低 |
| 3 | InGameView | 744行 | setter群（#1の副産物）／ ReticleView | -291行 | 低〜中 |
| 4 | InputManager | 477行 | PadInputReader | -220行 | 中 |
| － | Win32SelectWindowManager | 816行 | （保留） | － | － |

---

# 1. InGame（.cpp 1259行 + .h 339行）— 4つの責務が混在

## 責務の分解（実測）

| 行範囲 | 中身 | 行数 | 分離先 |
|---|---|---|---|
| 126-209 | 無名名前空間（`buildTabProjectileSetup` / `buildRainbowSetup`） | 84 | ① Setup |
| 213-412 | コンストラクタ（環境設定・View11個の生成＋setter呼び出し） | 200 | ① Setup |
| 540-816 | `setupSystems()` | **277** | ① Setup |
| 817-916 | `setupEvents()` | 100 | ② MissionProgress |
| 917-981 | `killRemainingEnemies()` / `spawnBoss()` | 65 | ② MissionProgress |
| 1233-1259 | `saveResultData()` | 27 | ② MissionProgress |
| 1025-1218 | `updateInventory()` 〜 `updateSwapSelection()` | 194 | ③ InventoryController |
| 413-464, 465-539, 982-1024, 1219-1232 | dtor・`loadResources()`・`spawnEntities()`・`updateInput()`・`update()`・`draw()` | 184 | InGame に残す |

① 561行 ／ ② 192行 ／ ③ 194行。合計 947行が移動対象で、残る本体は約190行（＋インクルード125行）。

シーンクラス本来の仕事は「ライフサイクルの委譲」だけなので、①②③を切り出す。

## 分離① InGameSetup — 最優先・効果最大

`setupSystems()` の277行（System登録の羅列＋順序コメント）、コンストラクタ内のView生成、ライティング／フォグ／シャドウマップ設定、無名名前空間の2関数は、すべて**構築時のみの処理でシーンの実行時状態と無関係**。

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

**注意**: System登録の順序コメント（「押し返しの前に」「AttackSystemの後に」等）は貴重な仕様ドキュメントなので、そのまま Setup 側へ移すこと。

## 分離② MissionProgress — ゲーム進行ルール

「雑魚IDを追跡 → 全滅でボス出現 → ボス撃破で残敵一掃 → リザルト保存」は勝敗仕様そのもの。EventBus 購読で完結しているため綺麗に切れる。

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

## 分離③ InventoryController

`updateInventory()` / `updateRenameTerminal()` / `updateSwapSelection()` 一帯は「入力を解釈して View に問い合わせ、イベントを発行する」典型的なコントローラ。InGame 本体とは PauseManager の共有以外に接点がない。

ゲームパッド対応で `updateInput()` という専用フックができたため、**移設先の呼び出し点は既に用意されている**。

```cpp
// game/ui/ingame/InventoryController.h
class InventoryController
{
public:
    void update();  // E/□/Esc/マウス/パッドの全処理
    [[nodiscard]] bool isOpen() const noexcept;
private:
    // m_isSwapMode, m_swapHeldIndex, m_wasMouseLeftDown をここへ
};
```

置き場所は `ui/ingame/`（InventoryView の隣）。View（描画）と Controller（入力解釈）が対になり追いやすくなる。

## 分離後の InGame（想定190行）

```cpp
InGame::InGame(...) : ...
{
    loadResources();
    spawnEntities();
    InGameSetup setup{ ... };
    m_context = setup.build(m_view);        // ← System登録・View生成・環境設定
    m_missionProgress.registerInitialEnemies();
}

void InGame::updateInput()
{
    m_context.m_battleStartSystem->pollAdvanceInput();
    m_inventoryController.update();
}

void InGame::update(float deltaTime)
{
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

## 実施時の注意

- **EventBus の破棄順**: InGame.h 先頭のコメント（購読者より前に EventBus を宣言）の制約は、MissionProgress 切り出し後も同じく残る。MissionProgress のメンバ宣言は EventBus より**後**（＝破棄が先）に置くこと
- System 登録の順序コメントは仕様ドキュメントとして Setup 側へ必ず引き継ぐこと

---

# 2. InventoryView（.cpp 1263行 + .h 483行）— 前回判定を撤回

前回は「責務は描画一本、座標計算の共有が逆に面倒になる」として分割不要と判断した。**その理由は右カラムの能力値ペインには当てはまらない**ことが分かったため、判定を撤回する。

## 根拠

`draw()` からの呼び出しは1行だけで、マス目の当たり判定（`m_slotBounds`）にも折り返し計算にも一切触れていない。

```cpp
drawStats(dividerX + padding, contentTop, rightWidth, playerId);
```

x / y / width / playerId を渡して終わる、独立した矩形領域である。

## 切り出す範囲

| 関数 | 行数 |
|---|---|
| `drawStats` | 101 |
| `drawMultiplierBadge` | 29 |
| `trackStatChanges` | 30 |
| `changeFlashAlpha` | 18 |
| `refreshBonusLabels` | 14 |
| 計 | **192** |

さらにこのペイン専用の状態が8つある。

```
m_previousStats / m_changeAmounts / m_hasPreviousStats / m_changeTime
m_labelMultiplier / m_bonusLabels / m_statLabels / m_statIconHandles
```

「前フレームの値と突き合わせて増減を拾い、拾った時刻から一定時間だけ光らせる」という**マス目の描画とは別のライフサイクル**を持っている。これが1クラスに同居していることが行数以上に読みにくさの原因になっている。

## 分離案

```
game/ui/ingame/
  InventoryStatsPane.h / .cpp   ← 右カラムの能力値表示と増減演出
```

```cpp
class InventoryStatsPane
{
public:
    InventoryStatsPane(core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen,
        core::ecs::ComponentManager& componentManager,
        core::iface::IResourceManager& resourceManager);

    /// @brief 能力値の一覧を描く
    void draw(int x, int y, int width, core::ecs::EntityId playerId);

    /// @brief 増減表示を消して比較の基準を取り直す（開くたびに呼ぶ）
    void resetChanges() noexcept;
};
```

InventoryView 側は `m_statsPane.draw(dividerX + padding, contentTop, rightWidth, playerId)` の1行になり、`resetStatChanges()` はそのまま委譲になる。**-200行超、ヘッダからもメンバ8個と Doxygen が消える。**

## 分割しないもの

`drawSection` / `drawSlot` / `drawHoldingPane` / `findSlotIndexAt` / `isLockedSlotAt` は `m_slotBounds` とレイアウト計算を共有しているため、**前回の判断どおり切らない**。切ると座標の受け渡しが増えて逆に読みにくくなる。

---

# 3. InGameView（.cpp 744行 + .h 456行）— 2種類の分離余地

| 行範囲 | 中身 | 行数 |
|---|---|---|
| 64-218 | `draw()` ＝ 描画順の決定 | 155 |
| 219-391 | **setter 28個の羅列** | 173 |
| 392-524 | モデル・影・装備武器の3D描画 | 133 |
| 525-642 | レティクル・チャージゲージ | 118 |
| 643-744 | 取得物・弾の3D描画 | 102 |

## (a) setter群 — InGameSetup の副産物として消える

InGameView の setter は 26 → **30個**に増えた。`registerSystem` → `setXxxSystem` のペア増殖が実際に進行している。

```cpp
auto* xxx = m_systemManager.registerSystem<XxxSystem>(...);
m_view.setXxxSystem(xxx);
```

「描画順は View が決め、描画内容は System が持つ」という設計判断自体は正しい。問題は配線コストだけなので、**分離①の `InGameContext` を丸ごと View へ渡す**ことで解決する。

```cpp
m_view.attach(context);  // setterの30呼び出しが1回に
```

.cpp から173行、.h から Doxygen 込みで約180行が消える。**独立作業にする必要はなく、分離①とセットで片付ける。**

## (b) 自前で描いている350行 — 方針の不揃い

PlayerHUD・EquipmentSlot・MiniMap・BossHUD・InteractPrompt などHUD要素は全て専用Viewクラスに切り出されているのに、**レティクルとチャージゲージだけ InGameView に直書きされたまま**になっている。

```
drawReticle / getAttackCooldownRatio / drawChargeGauge   ← 118行
```

`ui/ingame/ReticleView` として切り出せば他のHUD要素と方針が揃う。3Dモデル描画（`drawModels` / `drawShadowCasters` / `drawAttachedWeapon` / `drawExtensionPickups` / `drawProjectileModels` の235行）も同様に切れるが、こちらは「Viewがワールドを描く」という括りで一貫しているため急がない。

(a)(b) の両方を行うと InGameView は「描画順を決めるだけ」の約200行になり、クラス名と実体が一致する。

---

# 4. InputManager（.cpp 477行 + .h 226行）— パッド対応で倍増した

| 行範囲 | 中身 | 行数 |
|---|---|---|
| 35-152 | キーボード（キャプチャ・エッジ検出・consume） | 118 |
| 153-374 | **ゲームパッド** | 222 |
| 376-477 | マウス（座標・差分・カーソル表示） | 102 |

パッド部分は `capturePadFromXInput()` と `capturePadFromDirectInput()` の**2バックエンドを内包**しており、専用の状態も5つ持つ。

```
m_padKind / m_currentPadButtons / m_previousPadButtons
m_consumedPadButtons / m_padAxes
```

キーボード・マウス側とは `updateLastInputDevice()` 以外で状態を共有していない。

## 分離案

```
infrastructure/input/
  PadInputReader.h / .cpp   ← XInput / DirectInput の2系統とパッド状態
```

`IInputProvider` の実装面（`isPadButtonDown` / `consumePadPress` / `getPadAxis` / `isPadConnected`）は InputManager に残したまま委譲するだけなので、**呼び出し側は一切変わらない**。infrastructure 層で完結する。

優先度は中。今すぐ困ってはいないが、パッド周りを今後も触るなら先に切っておく価値がある。

---

# 5. Win32SelectWindowManager（816行）— 責務は2つだが今は触らない

| 中身 | 行数 |
|---|---|
| `createAllWindows()` ＝ 7つの窓のレイアウト＋生成 | 194 |
| WebView との JSONメッセージ ルーティング／ブロードキャスト | 約300 |

`handleDesktopMessage()`（140行）を中心としたメッセージ処理は、窓の生成・配置とは別の責務であり `SelectWindowMessageRouter` として切り出せる。

ただし **platform層でセレクト画面は既に完成しており、変更頻度が低い**。終盤の今、リスクを取って触る価値は薄いと判断する。

---

# 分割不要と判断したもの

| クラス | 行数 | 判断理由 |
|---|---|---|
| SettingsPanelView | 831 | `SettingsPanelController` が既に別クラスとして存在し、View/Controller 分離は完了済み。残りは `drawSpeakerIcon` 等の描画ヘルパーの羅列 |
| TitleView | 795 | `Title.cpp` が126行に収まっておりシーン側は薄い。パフォーマンスグラフの状態を持つ点だけ純粋なViewではないが、単一画面で変更頻度も低い |
| ModelRepository | 627 | 半分近くが json パース。気になるなら `ModelMetadataParser` として切れるが、Repository＝「読み込みと保持」の責務内。終盤の今は触らない |
| PlayerHUDView | 543 | HUDパネル1枚の描画。privateヘルパーに分割済み |
| BattleStartSystem | 496 | 演出の仕様が大きいだけで責務は単一 |
| MacAISystem | 488 | ボスFSMの仕様が大きいだけで責務は単一 |
| EquipmentSlotView | 453 | 装備スロット1つの描画。同上 |
| EnemyData / PlayerData | 339 / 289 | ゲッターの羅列で行数が出ているだけ。健全 |
| GameManager | - | 「シーン間で持ち回る共有データ」に収まっており問題なし |
| ServiceLocator | - | 多数箇所から使われるもの（IAudioManager 33箇所、IUIRenderer 27箇所）が中心で方針どおり。ICamera/IRenderer/IAnimator がロケータ登録なのに InGame へはコンストラクタ注入という二重経路だけ若干不揃いだが実害なし |

---

# 推奨作業順

| 順 | 作業 | 効果 | 備考 |
|---|---|---|---|
| 1 | **InGameSetup 切り出し** | InGame -561行／InGameView -173行 | setter は Context 一括渡しに置換。純粋な移動でリスク最小・効果最大 |
| 2 | **InventoryStatsPane 切り出し** | InventoryView -200行 | 1と依存関係なし。どちらから始めてもよい |
| 3 | **MissionProgress 切り出し** | InGame -192行 | イベント購読の移動。破棄順に注意 |
| 4 | **InventoryController 切り出し** | InGame -194行 | 呼び出し点は `updateInput()` が既にある |
| 5 | **ReticleView 切り出し** | InGameView -118行 | 方針を揃える作業なので後回し可 |
| 6 | **PadInputReader 切り出し** | InputManager -220行 | パッド周りを今後も触るなら |

1〜4はいずれも「コードの移動」であって書き換えではないため、終盤でも安全に実施できる。

**着手前の注意**: InventoryView.cpp / InventoryView.h に未コミットの変更が残っている場合は、先にコミットしてから移動を始めること。

---
---

# 付録: コードレビュー（ポインタ・所有権・パフォーマンス）

**状態: 4件とも未着手**（2026-08-20 時点で再確認）

対象：`src/`（約 47,000 行 / 403 ファイル）

## 総評

`new` / `delete` の直接使用はゼロ。所有権は `unique_ptr`、非所有は生ポインタ・参照で一貫して表現されており、設計は良好。以下は「さらに良くする」ための指摘。

| # | 問題 | 深刻度 | 修正コスト | 状態 |
|---|---|---|---|---|
| 1 | `ServiceLocator::provide` のポインタ調整漏れ | **高**（潜在） | 1 行 | 未着手 |
| 2 | `getAllEntities()` の毎フレームアロケーション | 中 | 中 | 未着手（呼び出し 56 → **64箇所**） |
| 3 | `has()` → `get()` の二重ハッシュ検索 | 低〜中 | 小（機械的） | 未着手（`has<` **134箇所**） |
| 4 | `ComponentArray::add()` の余分なコピー | 低 | 小 | 未着手 |

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

**場所**：`core/ecs/ComponentArray.h` / `ComponentManager.h`、呼び出し **64 箇所**
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

**場所**：`has<` の呼び出しが **134 箇所**。うち直後に `get<` が続くものが対象
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

| ファイル | 備考 |
|---|---|
| `system/ai/MeleeChaseAISystem.cpp` | 毎フレーム全敵 |
| `system/ai/EnemyRangedAttackSystem.cpp` | 同上 |
| `system/ai/DetectionSystem.cpp` | **2 組あり 4 回 → 2 回** |

`tryGet` は既に多数箇所で使われているため、方針は統一済み。取りこぼしの回収にあたる。

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

- `new` / `delete` の直接使用なし
- 所有＝`unique_ptr`・値、非所有＝`T*`・`T&` の使い分けが全体で一貫
- `ObjectPool` の `m_all`（`unique_ptr`）と `m_available`（`T*`）の分離
- `vector<unique_ptr<T>>` によるポインタ安定性の確保（`expand()` で借用ポインタが無効化しない）
- `EventBus::Subscription` による RAII での購読解除
- `InGame` のメンバ宣言順コメント（破棄順の明示）
- `ServiceLocator::clear()` の `m_order` による逆順破棄
- `get()` は assert、`tryGet()` は `nullptr` という意図の撃ち分け
- `[[nodiscard]]` の付与

---

## 付録の推奨着手順

1. **問題 1**（1 行。将来の地雷除去）
2. **問題 3**（機械的。AI 系のみ先行でも可）
3. **問題 4**（`add` 周りのみ）
4. **問題 2**（`forEach` 追加 → 呼び出し 64 箇所を段階的に移行）
