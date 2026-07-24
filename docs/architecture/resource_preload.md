# リソース先読み（ResourcePreloader）実装方針

起動直後の Bios シーンから3Dモデル・アニメーション・画像を少しずつ読み込み、
Loading シーンに到達した時点で全て完了させて InGame へ即座に遷移できるようにする。

マルチスレッドは使わず、メインループで**1フレームあたり時間予算内の分だけ**読み込む方式を採る。

---

## 現状の課題

### Loading シーンは実際には何もロードしていない

`Loading::update()` は `LoadingWindow` の演出完了通知（`notifyLoadingComplete()`）を待っているだけで、
リソースの読み込みは一切行っていない。

### 読み込みは InGame のコンストラクタで同期実行される

実際の読み込みは `SceneManager::changeScene(SceneType::InGame)` から呼ばれる
`InGame` のコンストラクタ内で一括実行される。

| 場所 | 読み込む内容 |
|---|---|
| `InGame::loadResources()` | Player モデル |
| `InGame::spawnEntities()` | Ground モデル、ステージ定義の敵モデル、Mac（ボス） |
| `InGame::setupSystems()` | タブ3種（Safari の弾）、RainbowWheel（Mac の弾） |
| 各 Factory / AnimationSystem | 上記モデルに紐づくアニメーション |

つまり画面が固まるのは「Loading 表示中」ではなく、
**Loading から InGame へ切り替わった瞬間の1フレーム**である。

---

## 方針：キャッシュを事前に温める

各 Repository は ID をキーにしたハンドルキャッシュを持っている。

- `ModelRepository::loadModelById()` … `m_modelHandles` にヒットすれば即返す
- `AnimationRepository::loadAnimationById()` … `m_handles` にヒットすれば即返す
- `ImageRepository::loadImageById()` … 同様

したがって **同じ ID で事前に load しておけば、InGame 側のコードは一切変更せずキャッシュヒットになる**。
`InGame` / 各 Factory / 各 System には手を入れない。

対象リソースは `assets/config/resources.json` に全て列挙されている（モデル9・アニメーション18・画像5）。
量が確定しているため進捗率も算出できる。

---

## マルチスレッドを使わない根拠

アセットの実測値は以下のとおり。

```
mv1 合計                31.3 MB / 27ファイル
  Player.mv1             7.55 MB
  Xcode.mv1              7.40 MB
  Mac.mv1                5.20 MB
  Safari.mv1             4.80 MB
  Tab × 3                1.85 MB ずつ
  RainbowWheel.mv1       0.37 MB
  アニメーション 18個      各 0.02〜0.07 MB
```

重いのは実質4ファイルのみで、アニメーション18個は合計でも 1 MB 未満。
1フレームに数個まとめて読んでも問題にならない。

一方、先読みに使える時間は Bios（`AUTO_ADVANCE_DELAY` 0.9秒 × 行数）＋ Lockscreen ＋
Title ＋ Select ＋ Loading の演出時間で数秒以上あり、
重い4件を1フレーム1件ずつ消化しても十分に間に合う。

**現状の問題は読み込み量ではなく、全てを1フレームに詰め込んでいること**であるため、
分散させるだけで解決する。スレッドを導入する理由がない。

---

## クラス設計

### 新規追加

`src/game/ResourcePreloader.h` / `ResourcePreloader.cpp`

`game/` 直下に置く。`GameManager` / `PauseManager` と同じ立ち位置であるため。

- Application が所有し、メインループから毎フレーム呼ぶ
- シーンをまたいで生き続ける
- Loading シーンが進捗を参照するので、`SceneFactory(gameManager, pauseManager)` と同じ注入経路に乗る

```cpp
namespace game
{
    /**
     * @brief 起動直後から少しずつリソースを先読みするクラス
     * @details Loading シーン到達時には読み込みが完了しているようにし、
     *          InGame への遷移でフレームが固まるのを防ぐ。
     */
    class ResourcePreloader
    {
    public:
        /**
         * @brief ResourcePreloader のコンストラクタ
         * @param resourceManager リソース管理インターフェース
         */
        explicit ResourcePreloader(core::iface::IResourceManager& resourceManager);

        /**
         * @brief 時間予算の範囲で先読みを進める
         * @param budgetMs このフレームで使ってよい時間（ミリ秒）
         */
        void step(float budgetMs);

        /**
         * @brief 全ての先読みが完了したかを返す
         * @return 完了していれば true
         */
        [[nodiscard]] bool isDone() const noexcept;

        /**
         * @brief 先読みの進捗を返す
         * @return 0.0〜1.0 の進捗率
         */
        [[nodiscard]] float getProgress() const noexcept;

    private:
        enum class Kind
        {
            Model,
            Animation,
            Image
        };

        struct Task
        {
            Kind        m_kind{};
            std::string m_id{};
        };

        core::iface::IResourceManager& m_resourceManager;
        std::vector<Task>              m_tasks{};
        std::size_t                    m_nextIndex{};
    };
} // namespace game
```

### `IResourcePreloader` は作らない

`ResourcePreloader` は Game層のロジックであり、DxLib にも Windows API にも触れない
（`IResourceManager` 越しにしか動かない）。
Infrastructure 層の実装を差し替える必要がないため、`core/interface` に抽象を切らない。

---

## 読み込み対象リストの構築

`IResourceManager` に ID 列挙用のメソッドを追加し、各 Repository が持つ map のキーを返す。

```cpp
[[nodiscard]] virtual std::vector<std::string> getAllModelIds()     const = 0;
[[nodiscard]] virtual std::vector<std::string> getAllAnimationIds() const = 0;
[[nodiscard]] virtual std::vector<std::string> getAllImageIds()     const = 0;
```

`ResourcePreloader` のコンストラクタでこれらを呼び、`m_tasks` を構築する。

この方式なら **`resources.json` にリソースを追加すれば自動的に先読み対象になる**ため、
追加漏れが原理的に起きない。

先読み対象を `game/constant/PreloadList.h` に手書きする案もあるが、
リソース追加のたびに手で追記する必要があり、漏れの温床になるため採らない。

---

## キューの並び順

`Player.mv1`（7.55 MB）はコライダー自動計算の全頂点走査（`MV1SetupReferenceMesh`）も走るため、
1件でも数百ミリ秒級のヒッチになり得る。
これが **Title / Select のような操作可能なシーンで起きるとカクつきが体感される**。

対策はスレッドではなく**キューの並び順**で行う。

- **重いモデルを先頭に置く** → 静止演出の Bios / Lockscreen のうちに消化される
- **軽いアニメーション・画像を後ろに置く** → Title / Select には軽い処理しか残らない

`IResourceManager` の ID 列挙をそのまま繋ぐのではなく、`ResourcePreloader` のコンストラクタで
`Model → Animation → Image` の順に `m_tasks` を組む。
モデルが最も重く画像が最も軽いため、この順序で自然に上記の並びになる。

---

## 呼び出し位置

`Application::run()` のループ内、`m_sceneManager->update()` の直前で `step()` を呼ぶ。

```cpp
// 起動直後から裏でリソースを温めておき、InGame 遷移時の固まりを防ぐ
m_resourcePreloader->step(PRELOAD_BUDGET_MS);

m_sceneManager->update(DELTA_TIME);
m_sceneManager->draw();
```

シーン側に処理を分散させないため、Bios / Lockscreen / Title / Select のどれを表示中でも
自動的に先読みが進み、シーン遷移の影響も受けない。

### 時間予算

`MV1LoadModel` は1モデルあたり数十ミリ秒かかるため、**実質「1フレーム1件」**になる。
`step()` は先に経過時間をチェックし、予算を超えていたら即座に return する
（1件読んだ結果として予算を超過するのは許容し、次のフレームに回す）。

Bios / Lockscreen は静止演出のため多少のカクつきは体感されないが、
Title / Select は操作があるので予算超過時に確実に中断する作りにする。

---

## Loading シーンの変更

完了条件に `isDone()` を加える。

- `notifyLoadingComplete()` が来ても先読みが未完なら `State::Loading` に留まる
- 先読み完了かつローディング演出完了で `startFadeOut()` へ進む
- `getProgress()` を進捗バーとして描画する

想定どおり動けば Loading 到達時点で `isDone()` は既に true になっており、
演出時間がそのまま待ち時間になる。先読みが間に合わなかった場合の保険としても機能する。

---

## 変更ファイル一覧

| ファイル | 内容 |
|---|---|
| `src/game/ResourcePreloader.h` / `.cpp` | **新規**。先読みキューと `step()` |
| `src/core/interface/IResourceManager.h` | ID 列挙メソッドを追加 |
| `src/infrastructure/ResourceManager.h` / `.cpp` | 各 Repository への委譲 |
| `src/infrastructure/repository/ModelRepository.h` / `.cpp` | ID 列挙の getter |
| `src/infrastructure/repository/AnimationRepository.h` / `.cpp` | ID 列挙の getter |
| `src/infrastructure/repository/ImageRepository.h` / `.cpp` | ID 列挙の getter |
| `src/Application.h` / `.cpp` | `ResourcePreloader` を所有し、ループ内で `step()` |
| `src/game/scene/SceneFactory.h` / `.cpp` | Loading へ preloader 参照を渡す |
| `src/game/scene/Loading.h` / `.cpp` | 完了条件に `isDone()` を追加、進捗バー描画 |
| `DxLib-3D.vcxproj` | 新規2ファイルの登録（ファイル列挙型のため手動追加） |

---

## 注意点

### 敵モデルの複製は先読みできない

`EnemySpawner::acquireModelHandle()` は `loadModelById()` の結果を
`MV1DuplicateModel` で複製して各個体に渡している。
先読みが温めるのは**複製元のベースハンドルまで**で、複製自体は spawn 時に走る。

複製は元データを共有するため軽いが、完全に0にしたい場合は
「ハンドルプールの事前充填」を第2段階として追加する。

### コライダー自動計算も前倒しされる

`ModelRepository::loadModelById()` は `colliderSize` が全て 0 のモデルに対して
`MV1SetupReferenceMesh` で全頂点を走査し AABB を求める。これは重い処理だが、
`loadModelById()` の中にあるため先読みで一緒に前倒しされる。

### DxLib の初期化後であること

`MV1LoadModel` / `LoadGraph` は DxLib の初期化完了後でなければ呼べない。
`ResourcePreloader` の生成は `ServiceLocatorInitializer::init()` の後に行う。

---

## 実装順序（コミット単位）

1. `IResourceManager` と各 Repository に ID 列挙を追加する
2. `ResourcePreloader` を追加する
3. `Application` に先読みを配線する
4. `Loading` シーンで先読み完了を待ち、進捗を表示する
