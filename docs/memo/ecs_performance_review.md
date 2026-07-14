# ECS活用度・C++パフォーマンスレビュー

作成日: 2026-07-12
対象: `src/` 全体（226ファイル・約13,000行を全読レビュー）

観点は次の2つ。

1. **ECSをしっかり活かせているか**（データ指向設計・System/Componentの責務分離）
2. **C++として不要なコピー・メモリ効率の悪い箇所がないか**

優先度の基準:

| 優先度 | 意味 |
|---|---|
| 🔴 高 | バグを生む・ECS設計の根幹に関わる。早めの修正推奨 |
| 🟡 中 | 毎フレームの無駄なコピー/確保。規模拡大時に効いてくる |
| 🟢 低 | 現状の規模では実害が小さいが、知っておくべき改善点 |

---

## 🔴 優先度: 高

### 1. `ComponentArray::get()` が未所持のComponentを暗黙に生成してしまう

**場所**: [ComponentArray.h:31-34](src/core/ecs/ComponentArray.h#L31-L34)

```cpp
T& get(EntityId id)
{
    return m_component[id];  // ← operator[] は無ければデフォルト構築して挿入する
}
```

`unordered_map::operator[]` はキーが無いとき**デフォルト構築した要素を挿入して返す**ため、
`has()` を確認せずに `get()` を呼ぶと「持っていないはずのComponent」が静かに生まれる。

実際に無チェックで `get()` している危険箇所:

- [AttackSystem.cpp:96](src/game/system/AttackSystem.cpp#L96) — `TagComponent` を無チェック取得（Tagを持たないターゲットに空Tagが生成される）
- [CollisionSystem.cpp](src/game/system/CollisionSystem.cpp) — `resolveCollision()` 内で `VelocityComponent` を無チェック取得（Groundは持っていないので、Player/Ground判定の組み合わせ次第で暗黙生成される）
- [InGame.cpp:204](src/game/scene/InGame.cpp#L204) — EnemyDeadEventハンドラで `HealthComponent` を無チェック取得

**問題点**: バグの温床（「持っている＝振る舞いが有効」というECSの前提が壊れる）＋不要なメモリ増加。

**修正案**: `get()` は `find()` + `assert`（またはポインタ返しの `tryGet()` を追加）にして、暗黙挿入を禁止する。

```cpp
T& get(EntityId id)
{
    auto it{ m_component.find(id) };
    assert(it != m_component.end() && "Component未所持のEntityにgetした");
    return it->second;
}
T* tryGet(EntityId id)  // has()+get() の2回検索も1回にまとまる
{
    auto it{ m_component.find(id) };
    return it != m_component.end() ? &it->second : nullptr;
}
```

---

### 2. Entityが一度も破棄されていない（destroy / removeAll が未使用）

**場所**: [EntityManager.h:31-34](src/core/ecs/EntityManager.h#L31-L34), [ComponentManager.h:80-86](src/core/ecs/ComponentManager.h#L80-L86)

`EntityManager::destroy()` と `ComponentManager::removeAll()` はプロジェクト内で**一度も呼ばれていない**。
敵死亡時は [InGame.cpp:188-216](src/game/scene/InGame.cpp#L188-L216) で `m_isVisible = false` にするだけ。

**問題点**:

- 死亡した敵のComponentが全て残り続け、`AISystem` / `AttackSystem` / `PhysicsSystem` / `CollisionSystem` が**死体を毎フレーム処理し続ける**（見えないだけで追跡・攻撃・衝突判定が動いている）
- 敵が増える設計（ダンジョン攻略）ではメモリ・CPUともに単調増加する

**修正案**: `EnemyDeadEvent` 受信時（またはアニメーション完了後）に `componentManager.removeAll(id)` + `entityManager.destroy(entity)` を呼ぶ「エンティティ破棄フロー」を作る。Systemのイテレーション中の削除を避けるため「破棄予約リスト→フレーム末尾で一括破棄」が安全。

---

### 3. EntityIdの再利用に世代（generation）がなく、破棄導入時に事故る

**場所**: [EntityManager.h:19-34](src/core/ecs/EntityManager.h#L19-L34), [Entity.h](src/core/ecs/Entity.h)

`m_recycledIds` でIDを再利用する設計だが、`Entity` はただの `uint32_t` ラッパーで世代カウンタがない。
問題2を修正して破棄を使い始めた瞬間、「古いEntityへの参照（例: `AIComponent::m_targetEntity`）が、再利用された**別の新Entity**を指す」バグが発生する。

**修正案**: EntityIdを `index + generation` の複合値にする（例: 上位8bitを世代に）。destroy時に世代をインクリメントし、参照時に世代不一致なら無効とみなす。もしくは当面IDを再利用しない（uint32_tなら60FPSで毎フレーム100体生成しても実用上枯渇しない）。

---

### 4. EventBusの `publish` が呼び出しごとにイベントをコピー（`std::any` 経由で2重コピー＋ヒープ確保）

**場所**: [EventBus.h](src/core/base/EventBus.h)

```cpp
m_listeners[key].push_back(
    [callback](const std::any& e)
    {
        callback(std::any_cast<TEvent>(e));  // ← ① 値キャスト＝コピー
    }
);
...
for (auto& listener : it->second)
{
    listener(event);  // ← ② TEvent → std::any への暗黙変換＝コピー（リスナーごとに毎回）
}
```

**問題点**:

- `listener(event)` の時点で `std::any` の一時オブジェクトが**リスナーの数だけ**構築される（イベント型がanyのSBOに収まらなければヒープ確保）
- `std::any_cast<TEvent>(e)` は**値キャスト**なのでさらにもう1回コピー
- `AttackHitEvent` は毎ヒットで発行され、購読者が4箇所（EffectSystem / HitEffectSystem / AudioEventListener / InGame）あるため、1ヒットで8回前後の無駄なコピーが発生
- `FileSlotChangedEvent` のように `std::string` を持つイベントはコピーのたびに文字列確保

**修正案**:

```cpp
// ① 参照キャストに変更（コピー除去）
callback(std::any_cast<const TEvent&>(e));

// ② publish側でanyを1回だけ構築
const std::any wrapped{ event };
for (auto& listener : it->second)
    listener(wrapped);
```

さらに踏み込むなら `std::any` をやめて `const void*` + 型ごとのリスナーリストにすればコピーゼロにできる。

---

### 5. EventBusに購読解除がなく、`this` キャプチャがダングリングする構造

**場所**: [EventBus.h](src/core/base/EventBus.h)、購読箇所: [EffectSystem.cpp](src/game/system/EffectSystem.cpp), [HitEffectSystem.cpp](src/game/system/HitEffectSystem.cpp), [AudioEventListener.cpp](src/game/event/AudioEventListener.cpp), [InGame.cpp:164-217](src/game/scene/InGame.cpp#L164-L217)

`subscribe()` はあるが `unsubscribe()` が存在しない。全購読者が `[this]` をキャプチャしている。
現状は「EventBusもSystemも同じInGameシーンのメンバで一緒に死ぬ」ため顕在化していないが、

- [SceneManager.cpp](src/game/scene/SceneManager.cpp) の遅延リセット中に旧シーンのEventBusへイベントが飛ぶ経路ができた瞬間にuse-after-free
- 「シーンをまたいで生きるEventBus」を将来導入したら即クラッシュ

**修正案**: `subscribe()` が購読トークン（ID）を返し、RAIIハンドル（デストラクタで自動解除）にする。最低でもヘッダに「EventBusは購読者より長生きしてはいけない」というライフタイム制約コメントを明記する。

---

### 6. 描画がECSを迂回している（RenderSystemが無く、シーンが個別IDを直接描画）

**場所**: [InGame.cpp:232-267](src/game/scene/InGame.cpp#L232-L267)

`InGame::draw()` が `m_playerId` / `m_groundId` / `m_enemyId` をハードコードで参照し、1体ずつ手描画している。
`RenderComponent` + `TransformComponent` は揃っているのに、それを走査するSystemが無い。

**問題点**:

- 敵を2体以上出した瞬間に描画されない（`m_enemyId` は最後の1体しか持てない）
- 「Componentを持つ者は自動で振る舞いが付く」というECSの利点が描画で完全に失われている
- デバッグ用コライダー描画（[InGame.cpp:252-260](src/game/scene/InGame.cpp#L252-L260)）もリリースビルドで常時実行される

**修正案**: `RenderSystem`（`RenderComponent` + `TransformComponent` を全走査して `IRenderer::drawModel`）を作り、`InGame::draw()` はそれを呼ぶだけにする。コライダー描画は `#ifdef _DEBUG` かデバッグフラグで分離。

---

### 7. InputSystem / MoveSystem / AnimationSystem が「単一Entity専用」になっている

**場所**: [InputSystem.h](src/game/system/InputSystem.h), [MoveSystem.h](src/game/system/MoveSystem.h), [AnimationSystem.h](src/game/system/AnimationSystem.h)

これらのSystemはコンストラクタで `EntityId` を1つ受け取り、その1体しか処理しない。
さらに問題が2つ重なっている:

- **MoveSystem**: `moveSpeed` をコンストラクタ引数で受けている。速度はデータなのでComponent（例: `MoveSpeedComponent`、または既存 `InputComponent` 側）に置くべき。現状、拡張子ボーナスで速度が変わる仕様なのにSystemが値を抱え込んでいる
- **AnimationSystem**: `PlayerAnimationState` 型とidle/walkハンドルがハードコードされ、**敵のアニメーションは誰も更新していない**（`Enemy` は `AnimationComponent<EnemyAnimationState>` を追加しているのに未使用）

**修正案**: 各Systemを「対象Componentを持つ全Entityの走査」に統一する。

- Input: `InputComponent` 保持者を走査（プレイヤーだけが持つので結果は同じ）
- Move: `InputComponent` + `VelocityComponent` + 移動速度データを走査
- Animation: `AnimationComponent<TState>` をテンプレートでなくす（状態enumを共通化するか、`m_animHandles` のマップをComponentに持たせる）ことで、プレイヤーも敵も同じSystemで回す

---

## 🟡 優先度: 中

### 8. `getAllEntities()` が毎フレーム・毎System呼び出しでvectorをヒープ確保して返す

**場所**: [ComponentArray.h:59-68](src/core/ecs/ComponentArray.h#L59-L68)、呼び出し8箇所（AISystem / AttackSystem×2 / CollisionSystem / PhysicsSystem / EffectSystem / HitEffectSystem / InGame）

毎フレーム、Systemごとに `std::vector<EntityId>` を新規確保→コピー→破棄している。
特に悪いのが [AttackSystem.cpp:57](src/game/system/AttackSystem.cpp#L57) で、**攻撃者ループの内側で** `getAllEntities<HealthComponent>()` を呼んでいるため、攻撃者数×ヒープ確保になる。

**修正案（段階的に）**:

1. すぐできる: `AttackSystem` のターゲット一覧をループ外に出す
2. 中期: `ComponentArray` に `forEach(callback)` を追加してvector生成自体をなくす
   ```cpp
   template<typename Func>
   void forEach(Func&& func)
   {
       for (auto& [id, comp] : m_component)
           func(id, comp);
   }
   ```
3. 長期: 問題9の密配列化とセットで解決

---

### 9. ComponentArrayが `unordered_map` ベースで、ECSの肝であるキャッシュ効率が出ていない

**場所**: [ComponentArray.h:71](src/core/ecs/ComponentArray.h#L71), [ComponentManager.h:89-96](src/core/ecs/ComponentManager.h#L89-L96)

- Componentが `unordered_map<EntityId, T>` に格納されており、メモリ上に散らばる。ECSの最大の利点である「連続メモリの線形走査」が得られていない
- さらに `ComponentManager::get<T>()` 1回につき、`typeid` → `type_index` のハッシュ検索 → `unordered_map` のEntity検索、と**ハッシュ検索が2段**入る。`getComponentArray()` 内は `find` + `operator[]` で同じマップを2回引いている
- Systemのホットループでは1Entityあたり `get()` を3〜5回呼ぶため、実質ハッシュ検索だらけになっている

**修正案**:

- 定番の **sparse set**（`std::vector<T> dense` + `std::vector<size_t> sparse`）に置き換えると、走査が連続メモリ・存在確認がO(1)配列参照になる
- 最低限の改善なら `getComponentArray()` の二重検索を `try_emplace` で1回にする:
  ```cpp
  auto [it, inserted] = m_componentArrays.try_emplace(type, nullptr);
  if (inserted) it->second = std::make_unique<ComponentArray<T>>();
  return static_cast<ComponentArray<T>*>(it->second.get());
  ```
- System側では、ループ前に `ComponentArray<T>*` を1回取得してループ内はそれを使う（型検索をループ外に追い出す）

---

### 10. `ComponentArray::add` / `ComponentManager::add` の値渡し→コピー代入で2回コピー

**場所**: [ComponentArray.h:21-24](src/core/ecs/ComponentArray.h#L21-L24), [ComponentManager.h:24-28](src/core/ecs/ComponentManager.h#L24-L28)

```cpp
void add(EntityId id, T component)   // ①呼び出し時にコピー
{
    m_component[id] = component;     // ②デフォルト構築＋コピー代入
}
```

`ComponentManager::add`（値渡し）→ `ComponentArray::add`（値渡し）→ コピー代入、で最大3回コピーされる。
`EffectComponent`（`std::vector` 持ち）や将来文字列を持つComponentで効いてくる。

**修正案**:

```cpp
void add(EntityId id, T component)
{
    m_component.insert_or_assign(id, std::move(component));
}
// ComponentManager側も
getComponentArray<T>()->add(id, std::move(component));
```

---

### 11. `SelectView::draw()` が毎フレーム `getJobInfo`（string入り構造体の値返し）＋ShiftJIS変換＋文字列連結

**場所**: [SelectView.cpp](src/game/scene/SelectView.cpp) `draw()`, [JobRepository.cpp](src/infrastructure/repository/JobRepository.cpp), [ResourceManager.h](src/infrastructure/ResourceManager.h)

`draw()` の中で毎フレーム:

- `m_resourceManager.getJobInfo(jobType)` — `JobInfo`（`std::string` を2本含む）を**値返し**でコピー
- `"職業: " + jobInfo.m_name` などの `std::string` 連結を約7本
- `converter->utf8ToShiftJis(...)` — WinAPI 2回（UTF-8→UTF-16→SJIS）＋ vector/string確保

つまり1フレームに10回以上のヒープ確保＋WinAPI往復が発生している。60FPSなら毎秒600回超。

同型の問題:

- [TitleView.cpp](src/game/scene/TitleView.cpp) `drawSplash()` — 毎フレーム文字列構築＋SJIS変換
- [LockscreenView.cpp](src/game/scene/LockscreenView.cpp) `draw()` — 日付・ヒント文字列を毎フレームSJIS変換
- [TitleView.cpp](src/game/scene/TitleView.cpp) `drawBackground()` — `std::to_string` +連結を毎フレーム3本

**修正案**: 「表示文字列は状態が変わった時だけ作る」を徹底する。ジョブ選択が変わったタイミング（`notifyJobSelected`）で変換済み文字列をメンバにキャッシュし、`draw()` は描くだけにする。`getJobInfo` は `const JobInfo&` 返しに変更（テーブルはメンバ配列なので参照で安全）。

---

### 12. `getMetadata()` が `ModelMetadata`（map2個＋string複数入り）を値返し

**場所**: [ModelRepository.cpp](src/infrastructure/repository/ModelRepository.cpp) `getMetadata()`, [IResourceManager.h](src/core/interface/IResourceManager.h)

`std::optional<ModelMetadata>` の値返しで、`stringProperties` / `floatProperties` の `unordered_map` ごと丸コピーされる。ロード時のみの呼び出しなのでフレーム毎の害はないが、[InGame.cpp:58,76](src/game/scene/InGame.cpp#L58) でコンストラクタと `loadResources()` の2回 `PlayerData::fromMetadata` している無駄と合わせて、シーン遷移のたびに数十回のmapコピーが走る。

**修正案**: `const ModelMetadata*` 返し（見つからなければnullptr）にする。あわせてInGameの二重初期化（メンバ初期化子リストでの `fromMetadata` は `loadResources()` で上書きされるので不要）を整理する。

---

### 13. CollisionSystemの重複コードと総当たり判定

**場所**: [CollisionSystem.cpp](src/game/system/CollisionSystem.cpp)

- `resolveCollision()` の「Player vs Ground」と「Enemy vs Ground」ブロックがほぼ完全に同じコード（約40行の重複）。Tagが増えるたびにコピペが増える構造
- `update()` は全コライダー総当たり O(n²)。現状4体なので問題ないが、敵を増やすと二乗で効く
- `isColliding()` と `resolveCollision()` で同じComponentを二度取得している（ハッシュ検索の重複、問題9と関連）

**修正案**: まず「動く物（Player/Enemy） vs Ground」の押し返しを1つの関数に統合（Tagで分岐せず `VelocityComponent` の有無で判定するとECS的にも綺麗）。敵数を増やす段階になったらグリッド分割等のブロードフェーズを検討。

---

### 14. `Title::exitApp()` の `std::exit(0)` がクリーンアップを全てスキップする

**場所**: [Title.cpp](src/game/scene/Title.cpp) `exitApp()`

`std::exit` はスタック巻き戻しをしないため、`ServiceLocator::clear()` も `DxLib_End()` も `Effkseer_End()` も呼ばれない。GPUリソース・サウンドハンドル・Win32リソース（`AddFontResourceEx` したフォント等）の解放が全て飛ぶ。

**修正案**: 「終了要求フラグ」を立てて `Main.cpp` のメインループを正常に抜ける経路にする（`ProcessMessage` ループの条件に組み込む）。

---

### 15. `std::function` の値渡し後にコピー格納している箇所

**場所**:

- [EventBus.h](src/core/base/EventBus.h) `subscribe()` — `callback` を値受けした後、ラムダに**コピー**キャプチャ（`[callback]`）。`[cb = std::move(callback)]` にすべき
- [Win32SelectWindowManager.cpp:14-27](src/platform/window/select/Win32SelectWindowManager.cpp#L14-L27) — コンストラクタで3本の `std::function` を値受けしてメンバに**コピー**代入。`std::move` すべき
- [Button.cpp](src/game/ui/Button.cpp) `setOnClick()` — 値受け後コピー代入。`m_onClick = std::move(callback);` にすべき

`std::function` のコピーは内包する呼び出し可能オブジェクトごとの再確保になるため、値受け＋moveの徹底が定石。初期化時のみのコストなので優先度は中の下。

---

## 🟢 優先度: 低

### 16. `InputManager` の `unordered_map` ベースのキー状態管理

**場所**: [InputManager.h](src/infrastructure/InputManager.h), [InputManager.cpp](src/infrastructure/InputManager.cpp)

- `m_previousKeyState` が `unordered_map<KeyCode, bool>`。キー数は固定なので `std::array<bool, KEY_COUNT>` で足り、ハッシュ検索も確保も消える
- `isKeyPressed()` 内の `m_previousKeyState[keyCode]`（mutableマップへの `operator[]`）は const メソッド内で挿入が起きる。動作は正しいが、意図しない挿入は問題1と同種の落とし穴
- `KEY_MAP` も `unordered_map` だが、`switch` か配列で分岐すればロード時確保も消える

### 17. `UIRenderer` のフォントキャッシュキーが毎回 `pair<string,int>` を構築

**場所**: [UIRenderer.cpp](src/infrastructure/UIRenderer.cpp) `drawText()` / `getTextWidth()`

テキスト描画のたびに `std::make_pair(m_currentFontName, fontSize)`（string コピー）でキーを作り、`std::map` を検索している。さらに `drawText()` は `find` の後 `m_fontHandles[key]` で計2〜3回検索。UI枚数が増えると効いてくる。フォント名は種類が少ないので「フォント名→ID変換＋ `unordered_map<int64_t, int>`（IDとサイズをパック）」程度で十分速くなる。

### 18. `EffectPool::isActive()` / `returnEffect()` の線形探索

**場所**: [EffectPool.cpp](src/infrastructure/EffectPool.cpp)

`m_activeSlots` を線形探索している。プールサイズが小さい現状は問題ないが、`EffectFactory::update()` → `std::erase_if(m_handleToType)` → 各ハンドルで `isActive` 線形探索、という組み合わせは O(ハンドル数×アクティブ数)。ハンドル→スロットの `unordered_map` を持てばO(1)になる。

### 19. `GroundFactory::getAllGrounds()` が毎回vectorを生成

**場所**: [GroundFactory.cpp](src/game/factory/GroundFactory.cpp)

呼び出し箇所は現状無いが、`const std::vector<std::unique_ptr<Ground>>&` を返すか削除してよい。同様に `EnemyFactory` は `m_enemies`（Actor本体）と `m_enemyIds` の二重管理になっており、Actorクラスは生成後に参照されないため `m_enemies` の保持自体が不要（ActorはEntity IDを返すだけの「セットアップ関数」に還元できる。これはECS的にも正しい方向）。

### 20. `AudioEventListener` のデッドコードと参照メンバ

**場所**: [AudioEventListener.h](src/game/event/AudioEventListener.h)

`AudioEventListener() = default;` は参照メンバ `m_eventBus` があるため実際には暗黙deleteされており、誤解を招くだけのデッドコード。削除推奨。

### 21. `Camera::update()` が引数を無視している（デバッグ固定のまま）

**場所**: [Camera.cpp](src/infrastructure/Camera.cpp)

`targetPosition` を受け取りながらコメントアウトされた固定カメラのまま。[InGame.cpp:224-226](src/game/scene/InGame.cpp#L224-L226) では毎フレーム `TransformComponent` を取得してカメラに渡しているので、意図と実装がズレている。また [InGame.cpp:225](src/game/scene/InGame.cpp#L225) の `enemyTransform` は未使用変数。

### 22. `Vector3` に演算子が不足しスカラー計算が手書きになっている

**場所**: [Vector3.h](src/core/utility/Vector3.h), 影響: [AISystem.cpp](src/game/system/AISystem.cpp), [PhysicsSystem.cpp](src/game/system/PhysicsSystem.cpp) 等

`operator*(float)` / `operator/` / `lengthSq()` / `normalize()` が無いため、各Systemで成分ごとの手書き計算が繰り返されている。コピー問題ではないが、`AISystem` の距離計算は `lengthSq()` があれば `sqrt` 自体を範囲判定から消せる（`AttackSystem` は既に距離二乗比較を実装済みで、`AISystem` と流儀が揃っていない）。

### 23. `IComponent` の名前が実態（ComponentArrayの型消去基底）と合っていない

**場所**: [IComponent.h](src/core/ecs/IComponent.h)

実際は「Component」ではなく「ComponentArrayの型消去インターフェース」。`IComponentArray` に改名した方が、Component（純粋データ構造体）とストレージの区別が明確になり、ECSの意図が伝わる。

### 24. `ObjectPool` の `getAvailableCount` 以外は良好

**場所**: [ObjectPool.h](src/core/ecs/ObjectPool.h)

設計は良い（初期化時 `Callbacks` を `std::move`、ポインタ返却でコピー無し）。細かい点として `initialize()` の `Config` は値渡しコピーだが、POD相当なので問題なし。置き場所が `core/ecs/` だが名前空間は `core::base` でズレているのでどちらかに揃えると良い。

---

## 総評

**良くできている点**:

- レイヤー分離は徹底されており、Game層からDxLib/WinAPIが漏れていない
- リソースは全リポジトリでハンドルキャッシュ＋デストラクタ解放が揃っている
- `EffectPool` / `ObjectPool` のプール設計、`AttackSystem` の距離二乗比較など、パフォーマンス意識のある実装が随所にある

**一方でECSとしての現状は「ECSのストレージを持ったシーン駆動ゲーム」に近い**。特に:

1. Systemの半分が単一Entity専用（問題7）
2. 描画がECSを迂回（問題6）
3. Entityのライフサイクル管理が未実装（問題2・3）
4. ストレージが `unordered_map` でデータ指向の恩恵がない（問題9）

「敵が複数出る」段階になる前に、🔴の1〜7を潰しておくと、以降の機能追加（敵の種類追加・ウェーブ制など）が「Componentを足すだけ」で済むようになる。面接向けアピールとしても「sparse set化・世代付きEntityId・RenderSystem化」は説得力のある改善ストーリーになる。

### 推奨着手順

1. 問題1（`get()` の暗黙挿入禁止）— 小さい変更でバグ予防効果が最大
2. 問題6・7（RenderSystem新設＋System汎用化）— ECSの形を正す
3. 問題2・3（エンティティ破棄フロー＋世代付きID）— 敵複数化の前提
4. 問題4・5（EventBusのコピー除去と購読解除）
5. 問題8〜11（毎フレームの確保・コピー削減）
6. 🟢は手が空いたときに
