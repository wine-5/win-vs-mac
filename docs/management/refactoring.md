# 総合レビュー: リファクタリング & パフォーマンス

対象: src 最新版（約48,000行 / 前回版との差分: CursorVisibility新設・パッド対応拡充・シーン管理変更・InputManager更新）
観点: ①責務分離（前回レビューの更新） ②パフォーマンス（新規・重点）

---

# 第1部 リファクタリング（前回からの更新）

## 解消済み

- **カーソル表示管理の散在（前回B-4）**: `CursorVisibility` として実装済み。「理由の立て下ろし＋毎フレーム合成」方式で、前回指摘した設計そのもの。ヘッダの「書き手はそれぞれ1か所に限る」というコメントも良い。**この項目はクローズ**。

## 継続（数値を最新版で更新）

| 項目 | 前回 | 今回 | 状態 |
|---|---|---|---|
| A-1. InGame の4責務混在 | 1259行 | **1341行** | 悪化。パッド対応で `updateSwapSelectionByPad/ByMouse` が入力コントローラ責務にさらに積まれた |
| A-2. TitleView の View+Controller 混在 | 795行 | **941行** | 悪化。パッド対応の入力分岐が View 内にさらに増加 |
| A-3. InGameView の setter 30個 | 30個 | 30個 | 変化なし |
| A-4. Win32SelectWindowManager の設定同期混在 | 816行 | 892行 | 微増 |
| B-1. 設定⇄JSON変換の platform 散在 | - | - | 変化なし |
| B-2. UI SE再生ヘルパの重複 | 4箇所 | 4箇所 | 変化なし |
| B-3. クリックのエッジ検出の自前実装 | 5箇所 | 5箇所 | 変化なし |

分離の設計案（InGameSetup / MissionProgress / InventoryController、InGameContext の一括渡し）は前回mdの内容がそのまま有効なので再掲しない。1点だけ追記:

- **InventoryController 切り出しの価値が上がった。** パッド対応で入力コントローラ責務が「マウス用」「パッド用」の2系統に分かれ（`updateSwapSelectionByPad` / `ByMouse`）、InGame 内の入力コードは前回比+80行。この2関数はまさにコントローラの中身なので、切り出せばそのまま持っていける。

---

# 第2部 パフォーマンス

先に総評: **メインループ・固定タイムステップ・先読みの設計は非常に良い**。accumulator方式＋処理落ち時の切り捨て（スパイラル防止）、先読み時間をフレーム計測から除外する処理、入力を1フレーム1回確定させる設計は、いずれも正しく実装されている。ここは触る必要がない。

問題は以下の順で影響が大きい。

## P-1.【最重要・リーク調査に直結】モデルハンドルが一度も解放されていない

Application.cpp に「1秒ごとに使用量を記録する（リーク調査用）」の一時コードがあるので、現在メモリ増加を調査中と推測する。その有力な原因候補を特定した:

**コードベース全体に `MV1DeleteModel` の呼び出しが1箇所も存在しない**（AnimationRepository のコメント「ここで MV1DeleteModel を呼んではいけない」を除き、grep でゼロ件）。`IResourceManager` にも unload / release 系の API がない。つまり:

1. **破壊可能ブロックの複製モデル**: `FactoryInitializer` がブロック1個につき `duplicateModel()` を呼ぶ（破壊状態を個体別に持つため）。InGame に入るたびに複製され、解放されない。**リトライ／周回のたびに段差状に増える**
2. **破砕モデル（m_fracturedHandle）**: 同上。ブロック数ぶん毎回複製
3. **敵モデルのプール（EnemySpawner::m_modelHandlePool）**: プール自体は良い設計（使い回しで生成コストを抑えている）が、EnemySpawner がシーンと共に破棄されるとき、プール内のハンドルを誰も削除しない

計測コードのコメントにある「カウントダウン明けに段差状に増えるのか」という仮説と、1（シーン入場時の一括複製）は整合する。

### 対処案

ModelRepository に複製ハンドルの台帳を持たせ、シーン単位で解放する:

```cpp
// ModelRepository
int duplicateModel(int modelHandle)
{
    const int duplicated{ MV1DuplicateModel(modelHandle) };
    if (duplicated != -1)
        m_duplicatedHandles.push_back(duplicated);  // 台帳に記録
    return duplicated;
}

void releaseDuplicates()  // InGame退出時（シーン遷移時）に呼ぶ
{
    for (const int handle : m_duplicatedHandles)
        MV1DeleteModel(handle);
    m_duplicatedHandles.clear();
}
```

- EnemySpawner のプールは、返却先が結局 duplicateModel 由来なので、台帳方式ならプール側の変更は不要（デストラクタでプールを空にするだけ。実体の削除は台帳が行う）
- ベースモデル（MV1LoadModel したもの）はキャッシュとして生かし続けて良い。増え続けるのは複製の方
- 対処後、既存の計測コード（core::probe）でリトライを数回繰り返し、水平になることを確認してから計測コードを削除する流れが安全

## P-2. ECSのコンポーネントアクセスコスト（毎フレームの基礎代謝）

ComponentArray は unordered_map 実装で、これはヘッダのコメントに設計判断として明記されている（数百体規模ではキャッシュ効率より単純さを優先、外部IFは差し替え可能）。**この判断自体は正しく、packed array への差し替えは不要**。ただし現行実装のまま安くできる箇所が3つある。

### P-2a. `getAllEntities()` が呼び出しごとに vector を新規確保

全60箇所（うちSystem内34箇所）が毎フレーム呼んでおり、**毎フレーム約40〜60回のヒープ確保**が発生している。ComponentManager に走査visitorを足せば確保ゼロにできる:

```cpp
// ComponentArray に追加
template <typename F>
void forEach(F&& func)
{
    for (auto& [id, component] : m_component)
        func(id, component);
}
```

利用側は `getAllEntities` のループを `forEach` に置き換えるだけで、ID列挙の確保と get() の再検索が両方消える（コンポーネント参照が直接渡るため）。60箇所を一括で変える必要はなく、毎フレーム呼ばれるSystemから順に置き換えれば良い。

注意: ループ内で `removeAll` / `destroy` する System（ProjectileSystem・EnemyDeathSystem 等）は走査中削除になるため、従来どおり ID列挙（getAllEntities）を使い続けるか、削除予約リストに積んでループ後に消す方式にする。

### P-2b. has() → get() の二重ハッシュ検索が残っている

`tryGet` が導入済みで、ComponentManager のコメントにも「2回ハッシュ検索する代わりに1回で済ませたい場面で使う」とあるが、System 側に古いパターンが残っている。例: MeleeChaseAISystem::update の

```cpp
if (!m_componentManager.has<component::ai::AIComponent>(entityId))
    continue;
auto& ai{ m_componentManager.get<component::ai::AIComponent>(entityId) };
```

は毎敵・毎フレームで2回検索している。`tryGet` への置き換えで半減する。同型のコードが AI 系・combat 系に十数箇所ある。

### P-2c. 型→ComponentArray の解決も毎回ハッシュ検索

`get<T>` のたびに typeid → type_index → unordered_map::find が走る。System は扱う型が固定なので、コンストラクタで `ComponentArray<T>*` を取得して持てば消せる（getComponentArray を public にするか、forEach 導入でまとめて解決するなら不要）。**優先度は a > b > c**。a と b だけで実測差が出るはずで、c は a を入れれば大半が不要になる。

## P-3. 毎フレームの ServiceLocator::get（23箇所）

PhysicsSystem・FootstepSystem・BattleStartSystem 等の update/draw 内で `ServiceLocator::get<IAudioManager>()` 等を毎フレーム呼んでいる。中身は type_index のハッシュ検索＋assert で、1回は安いが23箇所×毎フレームで積もる。

ServiceLocator 登録サービスはシーンより長生きすることが保証されている（シャドウマップの寿命コメントに明記あり）ので、**各Systemのコンストラクタで1回取得してメンバに保持**すれば安全に消せる。

## P-4. MiniMapView が毎フレーム全Entityを走査

draw() のたびに `getAllEntities<GroundSurfaceComponent>`（地形）と `getAllEntities<TagComponent>`（全Entity）を列挙し、座標変換して描いている。地形は動かないので:

- 地形のミニマップ座標は初回に計算してキャッシュし、ブロック破壊イベント（既存の BlockBreak イベント購読で可能）で該当分だけ無効化する
- 動くもの（プレイヤー・敵）だけ毎フレーム変換する

TagComponent の全列挙は「全Entityの中から敵とプレイヤーを探す」使い方なので、P-2a の forEach 化とあわせて EnemyTagComponent 等の絞り込み済みコンポーネントで回す方が筋が良い。

## P-5. drawModels の2パス走査

死亡ディゾルブの半透明を後回しにするため全Entityを2周し、各周で `has<ProjectileComponent>` と `has<DeathComponent>` を毎回検索している（Entity数×2周×2検索）。1周目で「不透明を描きつつ、ディゾルブ中のIDだけ小さな vector に積み、2周目はその vector だけ回す」形にすれば、2周目がディゾルブ中の数体だけになる。ディゾルブ対象は同時に数体なので効果は中程度だが、変更も小さい。

## P-6. 微小（気になったら程度）

- DamagePopupSystem::draw が表示中ポップアップ1件ごとに毎フレーム `std::to_string` している。char配列＋snprintf（PlayerHUDView と同じ方式）に揃えれば確保が消える。HUD側は既に snprintf で統一されており正しい
- ホットループ内の `core::log::info` はほぼコメントアウト済みで問題なし。ExtensionEquipSystem 等に残っているものはイベント時のみの発火なので放置で良い
- リーク計測の probe コード（std::format を60フレームに1回）は計測中は妥当。P-1解消の確認後に削除を忘れずに

## 問題なし（確認済み・変更不要）

| 箇所 | 確認内容 |
|---|---|
| メインループ | 固定タイムステップ＋accumulator、処理落ち時の切り捨て、先読み時間のフレーム計測除外、入力の1フレーム1回確定。すべて正しい |
| CollisionSystem | 乗る側×地面側に絞った上での総当たり。コメントどおり無駄な組み合わせを省いており、この規模で空間分割は不要 |
| 敵モデルのプール（EnemySpawner） | 使い回し設計は正しい（解放だけがP-1の問題） |
| シャドウマップ | プレイヤー周辺だけを写す範囲限定＋範囲外キャスターの事前除外。設計・実装ともに良い |
| AI系 | ターゲットを AIComponent に保持しており、毎フレームのプレイヤー探索はしていない |
| EventBus | 購読ID方式＋RAIIハンドル。dispatch は unordered_map 1回で、イベント発生はゲームイベント粒度なのでコスト問題なし |
| HUD文字列 | snprintf＋固定バッファで統一されており確保なし |

---

# 優先順位まとめ

| 順 | 作業 | 種別 | 効果 | 工数 |
|---|---|---|---|---|
| 1 | **P-1: 複製モデルの台帳＋シーン退出時解放** | 性能 | リーク解消（調査中の問題に直結） | 小 |
| 2 | A-1: InGameSetup / MissionProgress / InventoryController 分離 | 責務 | InGame 1341→約200行 | 中 |
| 3 | P-2a: forEach 導入（毎フレームSystemから順次） | 性能 | 毎フレームのヒープ確保 40〜60回→ほぼ0 | 小〜中 |
| 4 | P-2b: has→get を tryGet へ | 性能 | コンポーネント検索の半減（十数箇所） | 小 |
| 5 | P-3: ServiceLocator::get のコンストラクタ取得化 | 性能 | 毎フレーム検索23箇所の除去 | 小 |
| 6 | P-4: ミニマップの地形キャッシュ | 性能 | draw毎の全Entity走査の除去 | 小 |
| 7 | B-1〜B-3: SE集約・クリック消費API・SettingsJsonTranslator | 責務 | 重複の解消 | 小 |
| 8 | A-2: TitleView の分離 or リネーム | 責務 | 画面間の構造統一 | 小〜中 |
| 9 | P-5 / P-6: 2パス走査・to_string | 性能 | 微改善 | 小 |

P-1 だけは調査中の問題に直結するため最初に。責務系（2）と性能系（3〜6）は独立しているので並行して進められる。P-2〜P-4 はいずれも「実測で困ってから」でも遅くない類だが、変更が局所的でリスクが低いため、リファクタのついでに拾う価値がある。