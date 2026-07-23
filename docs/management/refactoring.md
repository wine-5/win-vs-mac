# リファクタリング評価レポート

- 評価日: 2026-07-23
- 対象: `src/` 一式（core / game / infrastructure / platform / ルート直下）
- 評価元: `docs/management/refactoring.md`（マスターリスト）セクション0の依頼形式に準拠
  - 各項目: **①現状 ②深刻度（高/中/低） ③改善案（コード例） ④オーバーエンジニアリング判断**
- 付録Aの24項目は再評価せず、**現況（対応済/部分対応/未対応）のみ**を末尾の表で確認
- マスターリストに無い新規発見（バグ疑い・重複・デッドコード）は「9. リスト外の新規指摘」に集約

---

## 総評サマリ

レイヤー分離は非常によく守られている。grepによる機械チェックの結果、**DxLibのincludeはinfrastructure層＋コンポジションルート（Main.cpp / Application.cpp）のみ**、**WinAPIはplatform層＋LogUtilのみ**、**game/coreからの逆流includeはゼロ**だった。設計ドキュメントとコードが一致している点は大きな強み。

一方で、今回のブランチで潰すべき本丸は次の4つ。

1. **EntityId再利用の世代（generation）不在が「実害化」している**（付録A3）。旧レビュー時と違い、現在は `destroy()` が実際に使われている（弾・敵）ため、再利用IDによる別Entity誤参照が理論上いつでも起こり得る状態。**最優先**。
2. **EventBusのunsubscribe不在**（付録A5）。現状はシーン所有のEventBusと購読者の寿命が一致しているため事故は起きていないが、`InGame::setupEvents` の `[this]` や各Systemの `[this]` は「たまたま安全」なだけで、構造上の保証がない。
3. **デッドコードが想定よりかなり多い**（1-1）。ファイル丸ごと未使用（SelectView）、クラス未使用（Label）、到達不能ステート（TitleのSplash系）、未使用インターフェースメソッド（drawBillboard, provideExisting 等）まで確認した。一覧を1-1に列挙。
4. **敵種判定ロジックの3重複**と**CollisionSystemの40行重複**（付録A13）など、「同じ知識が複数箇所にある」系の重複。敵を1種増やすと3〜4ファイル直す羽目になる。

逆に「やらなくてよい（オーバーエンジニアリング）」と判断したもの: 4-1のインクルード集約ヘッダ、4-4/8-3のDxLibラッパーヘッダ、8-1の「ISystemにComponentキャッシュ純粋仮想関数」、8-5のfadeのoptional化、7-1のシーンキャッシュ。理由は各項目に記載。

### 推奨着手順（ブランチ内の作業順）

| 順 | 作業 | 対応項目 |
|---|---|---|
| 1 | デッドコード一括削除（ビルドが軽くなり以降の差分が読みやすくなる） | 1-1, 1-2 |
| 2 | EntityIdの世代化 ＋ ComponentArray::get の暗黙生成廃止 | A3, A1 |
| 3 | EventBusの参照渡し化＋RAII購読トークン | A4, A5, 5-1, 5-2 |
| 4 | 敵種判定の一元化（EnemyTypeComponent導入） | 2-1, 9-4 |
| 5 | CollisionSystem統合・AttackSystemのクールダウン二重化解消 | A13, 7-5 |
| 6 | InGame::setupSystems の分割（Config構造体化） | 4-2, 4-3 |
| 7 | Vector3演算子追加→手書きベクトル計算の置換 | A22, 6-1 |
| 8 | 細かいパフォーマンス（A8〜A17系）を余力に応じて | 6-1, 付録A |

---

## 1. クリーンアップ（未使用コードの削除）

### 1-1. 未使用のメソッド・変数・クラス・ファイル・enumの洗い出し

**①現状**: grepによる呼び出し元探索で以下を確認した（宣言・定義以外の参照が0件のもの）。

**ファイル丸ごと未使用**
- `game/scene/SelectView.h` / `SelectView.cpp` — どこからもinclude・生成されていない。`Select` シーンはWin32ウィンドウ（`ISelectWindowManager`）方式に移行済みで、旧DxLib描画版のViewが残骸として残っている。**2ファイル削除可**。
- `game/ui/Label.h` / `Label.cpp` — `ui::Label` の生成箇所ゼロ。`IUIElement` 実装はButtonのみ使用。

**クラス内の到達不能コード**
- `Title` の `State::Splash` / `State::SplashFadeOut` と `m_splashTimer` / `m_dotTimer` / `m_dotCount` / `SPLASH_DURATION` / `DOT_INTERVAL` / `MAX_DOTS`、および `TitleView::drawSplash()`。初期状態が `TitleFadeIn` で、`Splash` へ遷移するコードパスが存在しない（コメントにも「Loading画面で起動演出済みのためスキップ」とある）。ステートマシンごと3状態（FadeIn/Idle/FadingOut）へ縮小できる。
- `Select` コンストラクタの引数 `inputProvider` / `fileProvider` / `fileEquipmentData` — 受け取るだけで保存も使用もしていない。SceneFactory側の呼び出しも合わせて削減可。

**メソッド・機能単位**
- `core::iface::IRenderer::drawBillboard()` ＋ `infrastructure::Renderer::drawBillboard()` — 呼び出し元ゼロ。
- `ServiceLocator::provideExisting()` — 使用ゼロ。
- `GroundFactory::getGroundById()` / `getAllGrounds()` / `getCount()` — 外部からの呼び出しゼロ（`getAllGrounds` は付録A19でも指摘済みの「使われないvector生成」そのもの）。
- `UIManager::clear()` — 呼び出しゼロ。
- `IAudioManager::stopBgm()` — game層からの呼び出しゼロ（BGMは常にplayBgmの上書きフェードで切替）。APIとして残す判断もあり得るが、YAGNIで削除推奨。
- `game/scene/Result.cpp` の `update(float deltaTime)` — `deltaTime` 未使用（`/*deltaTime*/` 化）。

**enum値・定数**
- `core::input::KeyCode::R` — コメントにも「現在未使用」。KEY_MAPのエントリごと削除可。
- `core::input::KeyCode::Tab` — 「ロックオン切り替え用」とあるがマッピングのみで参照ゼロ。実装予定が無いなら削除。

**構造体フィールド**
- `game::component::AIComponent` の `m_preferredDistanceMin` / `m_preferredDistanceMax` / `m_hoverHeight` — **読み取り箇所ゼロ**。同名フィールドは `RangeKeepAIComponent` に移設済みで、システムは全てそちらを読んでいる。AIComponent側は書き込みすらされていない完全な残骸。
- `game::data::PlayerData` の `m_modelPath` / `m_idleAnimPath` / `m_walkAnimPath` と対応getter — アニメはAnimationIdベース（`Player.cpp` で `loadAnimationById`）に移行済みで、パス系フィールドは詰めるだけで誰も読まない。
- `ServiceLocatorInitializer.cpp` のローカル変数 `resourceManagerPtr` — 取得後未使用（コンパイラ警告も出ているはず）。

**空の基底クラス**
- `game::factory::IFactory` — 仮想デストラクタのみで、`IFactory*` として扱う箇所ゼロ。コメント自身が「本来であればcreate関数を定義したいが引数が異なるため定義できない」と認めており、共通インターフェースとして機能していない。継承を外して削除するのが素直（Factory同士に共通の扱いが必要になった時点で再導入すればよい）。

**②深刻度**: 中（動作影響は無いが、量が多く「読まなくてよいコード」を読まされ続けるコスト＋リファク差分のノイズになる）

**③改善案**: 上記を1コミットで一括削除。削除順は「ファイル丸ごと → クラス/ステート → メソッド → フィールド/enum値」。各削除後にビルドが通ることだけ確認すればよい（テスト的な振る舞い変化はない）。

**④オーバーエンジニアリング判断**: 削除はオーバーエンジニアリングの逆なので全て実施推奨。唯一 `stopBgm` はインターフェースの対称性（play/stop）として残す判断も許容範囲。

### 1-2. `AudioEventListener() = default;` などデッドコード

**①現状**: **未対応**。`game/event/AudioEventListener.h` に `AudioEventListener() = default;` が残っている。参照メンバ `core::base::EventBus& m_eventBus` を持つため、このデフォルトコンストラクタは暗黙deleteされ、宣言として存在するだけの死んだ行（付録A20と同一指摘）。

**②深刻度**: 低（コンパイルは通るが、読み手が「デフォルト構築できるのか？」と一瞬迷う）

**③改善案**:
```cpp
// AudioEventListener.h
class AudioEventListener
{
public:
    // AudioEventListener() = default;  ← この行を削除するだけ
    AudioEventListener(core::base::EventBus& eventBus, core::ecs::EntityId playerId);
    ...
};
```

**④判断**: 1行削除。やらない理由がない。

---

## 2. クラスの責務違反

### 2-1. 各モジュール（Player / Enemy等）の区切り方の責務違反

**①現状**: 基本の区切りは良い。`EnemyBase`＋`EnemyBehaviors`（JSONレシピでコンポーネントを積む）への移行で敵種派生クラスは廃止済み、`EnemyFactory`（インスタンス化）と `EnemySpawner`（オーケストレーション）の分離も意図が明文化されていて健全。

ただし**「敵の種類は何か」という知識が3箇所に重複実装**されているのが最大の責務問題:

1. `InGame.cpp` setupEvents内 — `MacAIComponent`/`RangeKeepAIComponent`/`MeleeChaseAIComponent` の有無で "Mac"/"Safari"/"Xcode" を判定
2. `EnemyDeathSystem.cpp` の `inferEnemyType()` — 同じ判定で `EnemyType` を返す
3. `DetectionAlertVisualsSystem.cpp` の `enemyTypeName()` — 同じ判定で表示名を返す

「AIマーカーコンポーネントの有無＝敵種」という暗黙の対応表が分散しており、敵を1種追加すると3箇所＋`EnemySpawner::toModelId` を同時に直す必要がある。さらにこの判定は本来のセマンティクスと逆（「Macでも距離維持でもなければXcode」というフォールバック）で、AI構成を変えた瞬間に壊れる。

もう1点、`Win32SelectWindowManager.cpp`（platform層）が拡張子→`FileExtensionType` の解決とスロット状態管理を**FileEquipmentDataと二重に**実装している（拡張子の小文字化＋Resolver呼び出しがコピペ）。platform→gameのincludeは依存方向として合法だが、**同じ業務ルールを2箇所で維持**しているのが問題。

**②深刻度**: 中〜高（バグの温床。敵追加・AI構成変更時に確実に踏む）

**③改善案**: スポーン時に確定している事実（敵種）をコンポーネントとして持たせる。

```cpp
// game/component/EnemyTypeComponent.h（新規）
#pragma once
#include "game/constant/EnemyType.h"
namespace game::component
{
    /** @brief スポーン時に確定する敵の種類。表示名・撃破ログ・死に方分岐の唯一の情報源 */
    struct EnemyTypeComponent
    {
        constant::EnemyType m_type{ constant::EnemyType::Xcode };
    };
}
```
`EnemySpawner::spawn()` で `componentManager.add<EnemyTypeComponent>(id, { type })` を1行足し、3箇所の判定関数を `get<EnemyTypeComponent>(id).m_type` の参照に置換。`EnemyDeathSystem::isFallingDeath` も「RangeKeepでhoverHeight>0」というAI実装依存の推測から `type == Safari`（あるいはEnemyDataに `fallsOnDeath` フラグを持たせるとよりデータ駆動）へ寄せられる。

Win32SelectWindowManager側は、拡張子解決を自前で持たず `m_onFileSlotChanged` コールバック経由でGameManager側（FileEquipmentData）に一元化し、パラメータ表示に必要な値はコールバックの戻り値かGameManager参照で受け取る形へ。

**④判断**: EnemyTypeComponent追加はコンポーネント1個＋1行の書き込みで済み、削れるコードの方が多い。オーバーエンジニアリングではない。

### 2-2. 1クラスに2つの責務が混ざっていないか

**①現状**: 目立つのは以下。

- **`InGame`**: 「シーンのライフサイクル管理」に加えて (a) System登録時の弾リソース解決（タブ3種のモデルロード＋半径計算、レインボーの半径・中心計算）、(b) 撃破ログ・キル数・勝敗判定のゲームルール、(c) リザルトデータ構築、を1クラスで抱える。特に(a)はsetupSystemsを150行超に膨らませている主犯（→4-3で分割案）。
- **`AttackSystem`**: 「クールダウン管理」「ワインドアップ進行」「射程判定」「ダメージ解決」「死亡判定＆イベント発行」「開始エフェクトの種別決定ルール（Player近接ならSlash、敵は弾のみ…）」を1つのupdateで処理。最後の演出ルール分岐はデータ（ProjectileComponent::m_startEffectは既にその方向）へ寄せられる余地が大きい。
- **`Title`**: 到達不能なSplash演出ステートを抱えたまま（1-1参照）。削除すれば責務は明瞭になる。

**②深刻度**: 中

**③改善案**: InGameは4-3の分割案を参照。AttackSystemは「開始エフェクト決定」を関数に切り出すだけでも見通しが変わる:
```cpp
// AttackSystem.cpp 内
struct StartEffectDecision { bool play{}; core::constant::EffectType type{}; };
StartEffectDecision decideStartEffect(core::ecs::ComponentManager& cm,
    core::ecs::EntityId attackerId, constant::Tag tag, bool isProjectile);
```
将来的には「近接攻撃の開始エフェクト」もAttackComponentにEffectTypeを持たせてデータ駆動化するのが2-5と整合する。

**④判断**: クラス分割までは不要（関数抽出で足りる）。AttackSystemをさらにMeleeAttackSystem/ProjectileHitSystemへ分けるのは現規模ではオーバーエンジニアリング。

### 2-3. 描画と更新処理の混在（RenderSystem不在と関連）

**①現状**: **大きく改善済み・一部残**。旧レビュー時の「InGame::draw()にIDハードコード」は解消され、描画は `InGameView` に委譲、敵は `EnemyFactory::getEnemyIds()` の動的リスト、弾は `ProjectileComponent` 走査で描かれる。演出System（PlayerChargeVisuals / MacAwakenEffect / DetectionAlert / Telegraph系）も「updateで状態、drawで描画」を分け、Viewが描画順だけを決める構造になっており、これは良い設計。

残っているのは:
- `RenderComponent` を持つ全Entityを描く汎用RenderSystem（またはViewのdrawModels汎用化）が無く、`drawModels()` がplayer/ground/enemyの3経路をハードコードしている点。地形を増やす・NPCを足す等でまた分岐が増える。
- `EnemyRangedAttackSystem::applyAttackAnimation` が update内で `transform.m_scale` を直接書き換える演出（描画的関心が更新に混ざる）— ただしECS的には「Transformを書くのはSystemの仕事」なので許容範囲。

**②深刻度**: 低〜中

**③改善案**: InGameView::drawModelsを「RenderComponent＋TransformComponent全走査」に置き換える（描画順が問題になるならRenderComponentにlayer intを足してソート）:
```cpp
void InGameView::drawModels()
{
    for (auto id : m_componentManager.getAllEntities<component::RenderComponent>())
    {
        const auto& render{ m_componentManager.get<component::RenderComponent>(id) };
        if (!render.m_isVisible || render.m_modelHandle == -1) continue;
        if (m_componentManager.has<component::ProjectileComponent>(id)) continue; // 弾は専用描画
        const auto& tf{ m_componentManager.get<component::TransformComponent>(id) };
        m_renderer.drawModel(render.m_modelHandle, tf.m_position, tf.m_rotation, tf.m_scale);
    }
}
```
これで `draw(playerId, groundId, enemyIds)` の引数も不要になり、InGame→Viewの結合が細くなる。

**④判断**: 「RenderSystemクラス」を新設してSystemManagerに載せるのはやりすぎ（描画はupdate順と独立にViewが順序を握る現設計の方が扱いやすい）。**Viewのループ汎用化に留めるのが適量**。

### 2-4. ECSを活かせているか

評価済（付録Aの現況表を参照）。要点だけ: 「単一Entity専用System」はAnimationSystemが全走査化されて改善したが、**InputSystem / MoveSystemは依然playerId固定＋moveSpeedをSystemが保持**しており、A7は部分対応。

### 2-5. データ駆動になっているか

**①現状**: **かなり良い**。敵はbehaviors/animationsレシピ（JSON）、ボスFSMパラメータはmacData.json、弾はprojectileData.json、ステージ配置はstageData.json、当たり半径はradius=0でモデル実寸から自動計算、とデータ調整だけで変えられる範囲が広い。

データ駆動から漏れているもの:
- `ExtensionBonusCalculator` — 拡張子ボーナス値がconstexprハードコード。バランス調整のたびに再ビルド。
- `FileExtensionTypeResolver` — 拡張子→種別対応がハードコード。
- MacAISystemの `MELEE_LOCK` / `*_WINDUP` などアクション尺 — フェーズパラメータはJSONなのに、溜め時間・ロック時間だけソース側。ボス調整で一番触りたい値のはず。
- プレイヤー移動のダッシュ・ジャンプ力（PhysicsSystemの `DEFAULT_JUMP_FORCE` 等）。

**②深刻度**: 低〜中（ゲーム調整の反復速度に直結）

**③改善案**: macData.jsonに `windup` / `lockTime` をアクションごとに追加し `MacPhaseData` で読む。拡張子ボーナスは `extensionBonus.json` として `{"executable": {"atk": 10}, ...}` を定義してResourceManager経由で供給。

**④判断**: 「全定数をJSON化」は不要。**調整頻度が高い戦闘バランス値のみ**外出しするのが適量。演出用マジックナンバー（集中線の本数など）はソース内constexprのままで良い。

### 2-6. 拡張性・柔軟性

**①現状**: EnemyBehaviorsのレシピ方式、CameraEffectComponentの「チャンネル＋driver System」方式（新演出＝チャンネル1個＋System1個で既存に触れない、とコメントで規約化）は拡張に強い設計で高評価。

弱いのは:
- 敵種追加時の変更点が分散（2-1で解消可能）。
- behaviors名→インストーラのif-else連鎖（`installEnemyBehaviors`）。今は4種なので許容だが、増えるならmapへ:
```cpp
static const std::unordered_map<std::string_view,
    void(*)(core::ecs::ComponentManager&, core::ecs::EntityId, const data::EnemyData&)>
    INSTALLERS{
        { "meleeChase", installMeleeChase },
        { "rangeKeep",  installRangeKeep  },
        { "patrol",     installPatrol     },
        { "boss",       installBoss       },
    };
```
- `SceneFactory::createScene` の7分岐＋7本のunique_ptrメンバ。シーン追加のたびに3箇所（enum・create・reset）を編集する。`std::unordered_map<SceneType, std::unique_ptr<IScene>>` ＋ 生成関数テーブルにすると1箇所追加で済む。

**②深刻度**: 低

**④判断**: mapテーブル化は「増える見込みがあるなら」。現行の敵3種・シーン7種のままリリースするなら現状維持でも問題なく、先回りのテーブル化は軽いのでどちらでも可。

### 2-7. リファク関連TODOの設計案

→ セクション7で個別に回答。

---

## 3. アーキテクチャ違反

### 3-1. 依存の一方向性（逆流チェック）

**①現状**: **違反ゼロ（機械チェック済）**。
- `game/` `core/` から `infrastructure` / `platform` へのinclude: **0件**
- `core/` から `game` へのinclude: **0件**
- DxLib include: infrastructure層15ファイル＋Main.cpp＋Application.cpp のみ
- WinAPI(Windows.h): platform層＋`infrastructure/utility/LogUtil.h` のみ

グレーは2点:
1. **`Application.cpp` がDxLib直呼び**（`ProcessMessage` / `ClearDrawScreen` / `ScreenFlip`）。ApplicationとMainはコンポジションルートなので「例外」として明文化されていれば妥当だが、architecture.mdにその記述がない。
2. **`core/event/SelectEvents` 相当のイベントが `game/event/SelectEvents.h` に `namespace core::event` で定義されている** — ファイル配置(game)と名前空間(core)が不一致で、「フォルダ構成と名前空間の階層は一致させる」規約に違反。

**②深刻度**: 低（実害なし、規約の穴）

**③改善案**: (1) architecture.mdに「Main/Applicationはコンポジションルートとして全層へアクセス可」と1行追記するか、`IScreen` に `clearScreen()/flip()/processMessages()` を足してApplicationからDxLibを追い出す。(2) SelectEvents.hの名前空間を `game::event` に直す（発行側・購読側の修正は機械置換で済む）。

**④判断**: IScreen拡張は薄い割に綺麗になるので推奨。それ以上の抽象（IPlatformLoop等の新設）は不要。

### 3-2. Game層からのDxLib/WinAPI漏れ

**①現状**: **漏れなし**（3-1のgrep結果の通り）。`Lockscreen`View の `#ifdef _WIN32 localtime_s` は標準Cライブラリの範囲でありDxLib/WinAPIではない。合格。

### 3-3. LogUtil.h のアーキテクチャ違反（旧MEDIUM指摘）

**①現状**: **部分対応**。LogUtilは `infrastructure/utility/` に配置され `core::iface::ILogger` を実装する形になっており、Game層からの直接依存は解消済み。ただし2つ残る:

1. `LogUtil.h` が `<Windows.h>` をincludeしている。本プロジェクトの定義では **WinAPI依存はPlatform層の責務**（architecture.md: 「Platform層 Windows API実装」「Infrastructure層 DxLib・外部ライブラリ」）なので、コンソール制御(WinAPI)を使うロガー実装は本来 `platform/` にあるべき。しかも `<Windows.h>` が**ヘッダに**露出しており、LogUtil.hをincludeした翻訳単位全部に `min/max` マクロ等の汚染リスクを撒く。
2. `core/interface/ILogger.h` が `ServiceLocator.h` をincludeし、`LOG/LOG_W/LOG_E` **マクロ**を定義している。(a) core::ifaceの純粋インターフェースがcore::baseの具象機構に依存する、(b) naming_convention.mdの「マクロは基本的に使用禁止」に自己違反、(c) ILoggerを使うだけのコードにServiceLocatorが漏れる、の三重で筋が悪い。

**②深刻度**: 中

**③改善案**:
- LogUtil(.h/.cpp)を `platform/utility/` へ移動し、`Windows.h` includeと `HANDLE` メンバを**cppへ隠蔽**（pimplか、`void*`保持で可）。ServiceLocatorInitializerの登録includeを差し替えるだけで済む。
- マクロは可変引数テンプレート関数へ置換し、`core/utility/Log.h`（ILoggerとは別ファイル）に置く:
```cpp
// core/utility/Log.h
#pragma once
#include <format>
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
namespace core::log
{
    template<class... Args>
    void info(std::format_string<Args...> fmt, Args&&... args)
    {
        base::ServiceLocator::get<iface::ILogger>()->log(
            std::format(fmt, std::forward<Args>(args)...).c_str());
    }
    template<class... Args> void warn(std::format_string<Args...> fmt, Args&&... args);
    template<class... Args> void error(std::format_string<Args...> fmt, Args&&... args);
}
```
呼び出し側は `LOG("x={}", x)` → `core::log::info("x={}", x)` の機械置換。ILogger.hからServiceLocator依存とマクロが消え、規約とも整合する。

**④判断**: マクロ→関数置換は置換量こそ多いが機械的で、型安全性は同等（format_stringのコンパイル時チェックは関数版でも効く）。やる価値あり。ログレベルenum化やシンク抽象化まで広げるのはオーバーエンジニアリング。

### 3-4. worldToScreen の置き場所（7-2と同件）

**①現状**: **未対応**。`IRenderer::worldToScreen` にTODOコメント付きで残存。利用箇所は `ProjectileWindowSystem`（弾の実ウィンドウ配置）と `DetectionAlertVisualsSystem`（頭上バナー）の2箇所。

**②深刻度**: 低（責務の座りが悪いだけで実害なし）

**③改善案**: 判断基準を示す。**screenToWorld（クリック→ワールド）等の逆変換が必要になった時点で** `IViewProjection` を切るのが適切で、現状の「順変換2利用箇所のみ」では分離の便益が薄い。今回のブランチでは (a) TODOコメントを「IViewProjection分離はscreenToWorldが必要になったら」と条件付きに書き換えるだけ、または (b) どうしても座りを直したいなら最小分離:
```cpp
// core/interface/IViewProjection.h
namespace core::iface
{
    class IViewProjection
    {
    public:
        virtual ~IViewProjection() = default;
        virtual core::Vector3 worldToScreen(const core::Vector3& worldPos) const = 0;
    };
}
// infrastructure::Renderer が IRenderer と IViewProjection を多重実装し、
// ServiceLocatorに両キーで登録（provideExistingは未使用のため削除予定なら
// Renderer登録時に同一インスタンスをIViewProjectionとしてもproveする実装を追加）
```

**④判断**: **(a)推奨**。メソッド1本のためのインターフェース新設は現時点ではオーバーエンジニアリング寄り。

---

## 4. インクルード・肥大化まわり

### 4-1. インクルード専用ヘッダ（umbrella header）への集約

**①現状**: 集約ヘッダは存在しない。各ファイルが必要なものを個別includeしている。

**②深刻度**: —（問題ではなく提案の是非）

**③④改善案と判断**: **作らないことを推奨（オーバーエンジニアリングかつアンチパターン寄り）**。理由:
- umbrella headerは「本当は依存していないものへの依存」を隠し、変更時の再コンパイル範囲を最大化する（1ヘッダ変更で全翻訳単位が再ビルド）。
- 現状のinclude群は冗長に見えても「そのファイルが何に依存しているか」を正直に表しており、レイヤー違反のgrep検査（今回3-1で実施したもの）が機能するのはこのおかげ。
- ビルド時間が気になるなら、集約ではなく (a) 前方宣言の活用（InGame.hは既に一部実施済）、(b) MSVCなら `/std:c++20` のヘッダユニット or 従来型PCH（DxLib.h＋標準ライブラリのみをPCH化）が正攻法。PCHは「外部の安定ヘッダ」限定なら弊害が小さい。

### 4-2. InGameのinclude過多

**①現状**: `InGame.cpp` のincludeは約70本。内訳を見ると (a) 登録する全System 26本、(b) それらに渡すComponent/定数、(c) DEBUG系。**include自体は「実際に使っているもの」なので過多ではなく、setupSystemsが全Systemを知っていることの症状**。なお `IEffectFactory.h` と `RenderComponent.h` が**二重include**されている（実害なしだが整理対象）。`InGame.h` 側はメンバ型に必要なものが中心で妥当だが、`RenderComponent.h` はcppだけで足りる。

**②深刻度**: 低（コンパイル時間・見通しの問題）

**③改善案**: 4-3の分割とセットで解決する（System登録を別ファイルへ移せばincludeもそちらへ移る）。単独でやるなら、二重includeの削除＋InGame.hから `RenderComponent.h` を外す程度。

**④判断**: include数だけを目的にした対策（4-1）は不要。

### 4-3. InGame.cpp / InGameView.cpp の肥大化と分割ライン

**①現状**: `InGame.cpp` 468行。内訳: コンストラクタ＋DEBUG初期化 約60行 / loadResources 20行 / spawnEntities 45行 / **setupSystems 約150行** / setupEvents 60行 / update 50行（うち大半DEBUG） / draw 5行 / saveResultData 15行。`InGameView.cpp` は229行で、drawModels/drawReticle/drawProjectileModelsの3責務。**Viewは適量で分割不要**。

setupSystemsが太る原因は2つ:
1. System登録そのもの（26個、順序コメント付き）
2. **登録前のリソース解決**（タブ3種のロード＋半径計算、レインボーの半径・中心計算、ジョブ→SEタイプ変換）が混ざる

**②深刻度**: 中

**③改善案**: クラス分割ではなく「**リソース解決の関数抽出＋設定構造体化**」で十分。

```cpp
// InGame.cpp 内（無名名前空間 or privateメソッド）
struct RangedVisualSetup
{
    core::data::ProjectileMetadata m_meta{};
    std::vector<game::system::ai::RangedProjectileVisual> m_visuals{};
};
RangedVisualSetup buildTabProjectileSetup(core::iface::IResourceManager& res);

struct RainbowSetup
{
    core::data::ProjectileMetadata m_meta{};
    int m_handle{ -1 };
    float m_radius{ 0.0f };
    core::Vector3 m_center{};
};
RainbowSetup buildRainbowSetup(core::iface::IResourceManager& res);

core::constant::SeType resolvePlayerAttackSe(const data::JobSelectionData& job);
```
setupSystemsは「System登録の列」だけになり、80行前後まで縮む。さらに読みやすくするなら登録をフェーズごとのprivateメソッド（`registerCameraSystems()` / `registerCombatSystems()` / `registerVisualSystems()`）に3分割。

**分けすぎのライン**: 「SystemRegistrar」のような登録専用クラスの新設や、System登録順をデータ化する仕組みは**やりすぎ**。登録順は依存コメント（「CameraSystemより前」等）と一体で読める今の形が保守しやすく、順序をデータに逃がすと逆に追いにくくなる。

**④判断**: 関数抽出＝適量、クラス新設＝過剰。

### 4-4. DxLib.h直include → ラッパーヘッダ化（8-3と同件）

**①現状**: DxLib.hはinfrastructure層内部（cpp側）＋Main/Applicationのみ。**ヘッダへのDxLib漏れは無い**（唯一 `EffectPool.cpp` がEffekseerForDXLibをcpp内includeで、これも適切）。

**②深刻度**: 低

**③④改善案と判断**: **ラッパーヘッダは作らない（オーバーエンジニアリング）**。ラッパーの目的は通常「(a)マクロ汚染防止 (b)差し替え容易性」だが、(a)はDxLibがcpp内includeに閉じている現状で既に達成、(b)は `IRenderer` 等のインターフェース層こそが差し替え境界であり、DxLib関数を1:1で包む中間ヘッダは抽象の二重化にしかならない。Main.cppのコメント「自前ヘッダを先にinclude（マクロ衝突防止）」のような順序依存が気になるなら、その注意書きをdocsの規約（「DxLib.hはcppの最後にincludeする」）として明文化すれば足りる。8-3も同判断。

---

## 5. EventBus / ライフサイクル（評価済 → 現況確認のみ）

### 5-1. unsubscribe不在（=A5）

**未対応**。`EventBus` にsubscribeのみ。現状の安全性は「EventBusがInGameのメンバで、全購読者（各System・AudioEventListener・InGame自身）も同じInGameが所有し、シーン破棄で全員一緒に死ぬ」ことに依存している。**この不変条件はコードで表現されておらず**、例えば「ApplicationレベルのEventBusを足す」「Systemだけ作り直す」といった変更で即ダングリングになる。付録Aの改善案（購読トークン＋RAIIハンドル）のまま有効。実装するなら:

```cpp
// core/base/EventBus.h
class EventBus
{
public:
    class Subscription  // RAIIハンドル。破棄で自動解除
    {
    public:
        Subscription() = default;
        Subscription(EventBus* bus, std::type_index type, std::uint64_t id)
            : m_bus{ bus }, m_type{ type }, m_id{ id } {}
        ~Subscription() { reset(); }
        Subscription(Subscription&& o) noexcept { swap(o); }
        Subscription& operator=(Subscription&& o) noexcept { reset(); swap(o); return *this; }
        void reset()
        {
            if (m_bus) m_bus->unsubscribe(m_type, m_id);
            m_bus = nullptr;
        }
    private:
        void swap(Subscription& o) noexcept
        {
            std::swap(m_bus, o.m_bus); std::swap(m_type, o.m_type); std::swap(m_id, o.m_id);
        }
        EventBus* m_bus{ nullptr };
        std::type_index m_type{ typeid(void) };
        std::uint64_t m_id{ 0 };
    };

    template<typename TEvent>
    [[nodiscard]] Subscription subscribe(std::function<void(const TEvent&)> callback)
    {
        const auto key{ std::type_index(typeid(TEvent)) };
        const auto id{ m_nextId++ };
        m_listeners[key].push_back({ id,
            [cb = std::move(callback)](const void* e)
            { cb(*static_cast<const TEvent*>(e)); } });
        return Subscription{ this, key, id };
    }

    template<typename TEvent>
    void publish(const TEvent& event)  // 5-2: コピー・any・ヒープ確保なし
    {
        const auto it{ m_listeners.find(std::type_index(typeid(TEvent))) };
        if (it == m_listeners.end()) return;
        for (auto& l : it->second) l.m_fn(&event);
    }

private:
    struct Listener { std::uint64_t m_id; std::function<void(const void*)> m_fn; };
    void unsubscribe(std::type_index type, std::uint64_t id)
    {
        auto it{ m_listeners.find(type) };
        if (it == m_listeners.end()) return;
        std::erase_if(it->second, [id](const Listener& l) { return l.m_id == id; });
    }
    std::unordered_map<std::type_index, std::vector<Listener>> m_listeners;
    std::uint64_t m_nextId{ 1 };
};
```
各購読者は `core::base::EventBus::Subscription m_sub;` をメンバに持つだけ（複数なら `std::vector<Subscription>`）。**注意**: publish中のunsubscribe（コールバック内でシーン遷移→System破棄）に備え、厳密にはerase遅延かインデックスループが要る。現状もpublish中にsubscribeし得る構造（コンストラクタ購読なので実際は起きない）なので、導入時に「コールバック内での購読/解除は禁止」とコメント規約にするのが現実的。

**オーバーエンジニアリング判断**: 今の所有構造が変わらない前提なら「未対応のまま＋不変条件をEventBus.hのコメントに明文化」でも許容。ただし今回まとめて直すブランチなら、5-2（下記）と同時に上記へ差し替えるのが効率的。

### 5-2. publishのイベントコピー・ヒープ確保（=A4）

**未対応**。現行は `std::any_cast<TEvent>(e)`（参照でなく**値**キャスト）のため、**リスナー1人ごとにイベントの完全コピー**が発生し、`std::string` を持つ `FileSlotChangedEvent` などはヒープ確保も伴う。最低限の1行修正でも効果あり:
```cpp
callback(std::any_cast<const TEvent&>(e)); // 値→const参照キャストへ
```
根本対応は5-1のコード例（`const void*`渡し）で、any構築自体も消える。

---

## 6. メモリ・パフォーマンス

### 6-1. このリファク機会にまとめて直すべきか（付録A以外の新規発見を含む）

**①現状と新規発見**: 付録Aの24項目以外に、今回の精査で見つけたもの:

- **`AttackSystem::resolveAttack` が攻撃者ごとに `getAllEntities<HealthComponent>()` を呼ぶ**（vector確保）。弾はProjectileSystemにより毎フレームattackRequestedが立つため、**弾の数×フレームで毎回確保**が発生する。A8の「ループ内取得」は形を変えて残存。
- **`ProjectileSystem` の `std::ranges::find(m_pendingDestroy, id)`** — 弾数×破棄予約数の線形探索。弾が少ない現状は無害だが、ノヴァ（全方位弾）で数十発出る設計なので注視。
- **`TargetingSystem`** — aimer×候補の二重ループで毎フレーム `getAllEntities` 2回。aimerは実質プレイヤー1体なので現状軽微。
- **`MacAISystem::countAliveMinions`** — アクション抽選時のみ呼ばれ頻度が低く問題なし（良い実装）。
- **`DetectionAlertVisualsSystem::drawBanner`** — 毎フレーム `std::string` 構築＋SJIS変換（"たった今"等）。A11と同種。表示中のみなので優先度低だが、変換済み文字列をコンストラクタでキャッシュ可能。
- **`Renderer::m_originalColors`（unordered_map<int, vector>）** — 使い方は正しく、resetでeraseされるためリークなし。問題なし。
- **`EffectPool::update`** — activeスロット全走査で `IsEffekseer3DEffectPlaying` を毎フレーム呼ぶ。エフェクト数十個規模なら許容。

**②深刻度**: 全体として中。60FPS維持を脅かす単独の爆弾は無いが、「毎フレームの小さなヒープ確保」（getAllEntitiesのvector、文字列生成、EventBusコピー）が面で積もっている。

**③改善案（まとめて直す場合の順序）**:
1. EventBusの参照化（5-2）— 1行で効く。
2. `ComponentArray::forEach` の追加でgetAllEntitiesのvector確保を撲滅:
```cpp
// ComponentArray.h
template<typename Fn>  // Fn: void(EntityId, T&)
void forEach(Fn&& fn)
{
    for (auto& [id, comp] : m_component) fn(id, comp);
}
// ComponentManager.h
template<typename T, typename Fn>
void forEach(Fn&& fn) { getComponentArray<T>()->forEach(std::forward<Fn>(fn)); }
```
   注意: **ループ中のadd/removeは不可**になるため、現在スナップショット前提で書かれている箇所（DetectionAlertVisualsSystem::updateの自己remove、EnemyDeathSystem/ProjectileSystemの破棄）は「削除予約→ループ後に実行」へ書き換えてから移行する。移行が面倒な箇所はgetAllEntitiesのままでもよい（毎フレーム大量に回るPhysics/Attack/AI系だけforEach化するのが費用対効果最大）。
3. AttackSystemはupdate冒頭で `targets` を1回だけ取得してresolveAttackへ渡す。
4. A11/A16/A17（文字列キャッシュ・キー配列化・フォントキー）は余力枠。

**④判断**: 2までは今回やる価値が高い。sparse set化（A9）は現在のEntity数（プレイヤー1＋敵十数体＋弾数十）では**体感差が出ないため今回は見送りが妥当**（ストレージ構造の全面変更はバグリスクの割に効果が薄い）。60FPSを割る兆候が出た時の切り札として残す。

### 6-2. 詳細24項目

付録Aの現況表（末尾）を正とする。

---

## 7. ソース内TODO

### 7-1. SceneFactory: シーン生成のたびのnew、将来のキャッシュ方針

**①現状**: 遷移のたびに `make_unique` で作り直し、旧シーンは1フレーム遅延で `resetScene`（use-after-free対策コメントあり）。軽量シーンなので実測上問題なし。

**②深刻度**: 低

**③④改善案と判断**: **キャッシュは導入しない（オーバーエンジニアリング）**。作り直し方式には「シーン状態のリセット漏れが構造的に起きない」という強い利点があり、TODOコメント自身が心配している「reset()関数を持たせる必要」はキャッシュを導入した瞬間に発生する新たな義務。InGameのロードが遅くなったら「リソース側（ResourceManager）のキャッシュを温める」方向で解決すべきで、シーンオブジェクトの再利用ではない。TODOは「方針決定: 作り直し方式を維持。重くなったらリソースキャッシュで対処」と書き換えて完了にすることを推奨。

### 7-2. worldToScreen の責務

→ 3-4と同一。**(a)現状維持＋TODOの条件明記を推奨**。

### 7-3. PlayerData: パラメータ直接変更→武器の攻撃力方式へ

**①現状**: `applyExtensionBonus` / `applyJobParameters` がPlayerDataの実値を直接加算・上書きする。ベース値と職業加算値を分離して保持する下地（m_base* / m_job*Addition）は既にある。

**②深刻度**: 低（現仕様では正しく動く。UI表示で「基礎値＋ボーナス」を分けて見せたくなった瞬間に困る構造）

**③改善案**: フル装備システムを作るのではなく、**「最終値＝ベース＋修正の合算」を計算で出す**形に留める:
```cpp
// game/data/StatModifier.h（新規・FileExtensionBonusを一般化）
struct StatModifier { float hp{}, atk{}, def{}, spd{}, attackRange{}; };

// PlayerData: 実値フィールドを持たず、都度合算
class PlayerData
{
public:
    void addModifier(const StatModifier& m) { m_modifiers.push_back(m); }
    [[nodiscard]] float getAttackPower() const noexcept
    {
        float v{ m_baseAtk };
        for (const auto& m : m_modifiers) v += m.atk;
        return v;
    }
    // 他ステータスも同様
private:
    std::vector<StatModifier> m_modifiers{};
};
```
「武器」という概念のクラス化は、武器が装備品として付け替えられる仕様が入るまで不要。

**④判断**: 上記の範囲なら適量。WeaponクラスやEquipmentSlotの新設は現仕様ではオーバーエンジニアリング。

### 7-4. AttackStartEventにSEを紐付け

**①現状**: TODOコメントのまま。AttackHitEventは既に `m_seType` を持ち、AudioEventListenerが再生する仕組みが完成している。

**②深刻度**: 低（純粋な機能追加）

**③改善案**: 既存パターンの完全な相似形で実装できる:
```cpp
// InGameEvents.h
struct AttackStartEvent : public core::event::IGameEvent
{
    core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };
    core::constant::EffectType m_effectType{ core::constant::EffectType::None };
    core::constant::SeType m_seType{ core::constant::SeType::None }; // 追加
    ...
};
// AttackSystem: startEffect決定と同じ分岐でseTypeも決める
//   （敵の弾はProjectileComponentにm_startSeを持たせればデータ駆動になり、
//     projectileData.jsonの "startSe" で敵ごとに音を変えられる）
// AudioEventListener: subscribe<AttackStartEvent>を1本追加してplaySe
```

**④判断**: 適量。SEの「敵種ごと変更」はProjectileMetadata/EnemyData経由にすると2-5の方針とも一致。

### 7-5. AIComponent::m_attackCooldown の削除可否

**①現状**: 判断材料が揃った。現在の攻撃クールダウンは**二重ゲート**になっている:
- `AIComponent::m_attackCooldown/m_currentAttackCooldown` — MeleeChaseAISystem・RangeKeepAISystem（接触攻撃）が「攻撃を要求するか」のゲートに使用
- `AttackComponent::m_attackCooldown/m_currentCooldown` — AttackSystemが「要求を実行するか」のゲートに使用
- ボスは `installBoss` で後者を0にして無効化（＝二重化の弊害を回避するための特例が既に発生している）

両方JSON由来の同名キー `attackCooldown` を読んでおり、片方だけ調整すると「AIは要求するがAttackSystemが弾く」不可解な挙動になる罠がある。

**②深刻度**: 中（調整時の罠）

**③改善案**: **AttackComponent側に一本化してAIComponentから削除**する。AI Systemは「レンジ内なら毎フレーム `m_attackRequested = true`」だけを行い、間隔制御はAttackSystemに任せる（弾のProjectileSystemが既にこの方式で、統一される）。MeleeChaseの「攻撃した瞬間だけAttack1アニメ要求」はAttackStartEvent購読（7-4で拡張するイベント）か、AttackComponentに `m_justFired` フラグを立てて読む形で代替できる。RangeKeepの遠距離発射は別系統（m_fireCooldown）なので影響なし。

**④判断**: 削除する方向で確定してよい。installBossの特例コードも消せて負債が減る。

---

## 8. todo.md移設のリファク項目

### 8-1. ISystemに「初回にComponentをキャッシュする純粋仮想関数」を追加

**①現状**: 未実装。各Systemはupdate内で毎回 `get<T>()`（ハッシュ検索）している。

**②深刻度**: —（提案の是非）

**③④改善案と判断**: **この形では実装しないことを推奨（オーバーエンジニアリング＋危険）**。理由:
- Entityは実行時に増減する（弾・召喚）ため「初回にキャッシュ」は成立しない。キャッシュ更新の通知機構を作り込むとECSフレームワーク自作の泥沼に入る。
- `unordered_map` はノードベースで要素参照自体は安定だが、remove後の参照は普通にダングリングする。ライフサイクル（A2/A3）を整備中の今、生参照キャッシュは最悪の相性。
- 同じ目的（ハッシュ検索の削減）は6-1の `forEach`（イテレーション中にComponent参照が手に入る）で、**Systemに状態を持たせずに**達成できる。二重検索が残る「別Componentの随伴取得」（TransformとVelocityを両方触る等）は、頻度の高いSystemだけ `tryGet` ポインタを使う:
```cpp
// ComponentArray.h に追加（A1の改善と同時に）
T* tryGet(EntityId id)
{
    auto it{ m_component.find(id) };
    return it != m_component.end() ? &it->second : nullptr;
}
```
`has()`→`get()` の二連ハッシュ検索が `tryGet()` 1回になる。これで8-1の動機は満たされる。

### 8-2. 初期化は `{}`、代入は `=` に統一

**①現状**: 初期化の `{}` はほぼ全域で統一されており優秀。**代入側で `= {};` を使っている箇所は8件、全て `MacAISystem.cpp` の `velocity = {};`**。

**②深刻度**: 低（可読性の好みの問題）

**③改善案**: MacAISystemに停止ヘルパを1個作ると、規約対応と重複削減が同時に済む:
```cpp
// MacAISystem.cpp 無名名前空間
void stopMovement(core::ecs::ComponentManager& cm, core::ecs::EntityId id)
{
    if (auto* v = cm ... has/get VelocityComponent ...)  // tryGet導入後はtryGetで
        v->m_velocity = core::Vector3{ 0.0f, 0.0f, 0.0f };
}
```
8箇所の `if (hasVelocity) ... = {};` パターンが `stopMovement(m_componentManager, entityId);` に潰れる。

**④判断**: 適量。プロジェクト全体をリントで縛るほどの件数ではない。

### 8-3. DxLibラッパーヘッダ

→ **4-4と同一。作らない判断を推奨**。

### 8-4. 選択した職業・ファイルの記憶と再挑戦時の復元

**①現状**: **未対応（データは残っているがUIに反映されない）**。`GameManager`（Application所有）が `JobSelectionData` / `FileEquipmentData` をシーンをまたいで保持しているため**データ自体は再挑戦時も生きている**。実際、Result→Select→再スタートでInGameは前回の職業・ファイルボーナスを適用する。問題はSelect側で、`Win32SelectWindowManager` が生成時に `m_jobSelected{false}` / スロット空で始まり、**ウィンドウ表示が前回選択を反映しない**（さらに `notifyGameStart` は `isJobSelected()` がwindowManagerローカルフラグなので、データ上は選択済みでも「職業を選択してください」警告が出る）。

**②深刻度**: 中（UX仕様として明確に不整合）

**③改善案**: SceneFactoryのSelect生成時に既存選択を注入する。
```cpp
// ISelectWindowManager にリストア用APIを追加
virtual void restoreSelection(core::constant::JobType jobType, bool hasJob,
    const std::array<std::string, 3>& filePaths) = 0;

// SceneFactory::createScene(Select) の windowManager 生成後:
const auto& job{ m_gameManager.getJobSelectionData() };
const auto& files{ m_gameManager.getFileEquipmentData() };
std::array<std::string, 3> paths{};
for (int i{ 0 }; i < data::FileEquipmentData::MAX_SLOTS; ++i)
    if (files.hasSelection(i)) paths[i] = files.getFilePath(i);
windowManager->restoreSelection(job.getSelectedJobType(), job.hasJobSelected(), paths);
```
Win32SelectWindowManager側はrestoreで `m_jobSelected` / `m_slotPaths` / `m_slotExtTypes` を埋め、JobWindow・FileSelectWindowへ初期選択を反映するメッセージを送る。2-1で指摘した拡張子解決の重複解消と同時にやると差分が綺麗。

**④判断**: 適量（仕様実装）。「前回選択の永続化（ファイル保存）」まで広げるのは別仕様なので今回はやらない。

### 8-5. Title/Select::m_fade の std::optional化、PlayerFactory::getPlayer() にassert

**①現状**:
- `m_fade` は両シーンとも `std::unique_ptr<FadeTransition>` で、nullptrが「フェード無し」を表現している。**unique_ptrは既にoptional相当の意味論を持つ**ため、optional化の実益は「ヒープ確保が消える」ことのみ。ただし現状 `Title::update` の `FadingOut` ケースは `m_fade->update()` をnullチェック無しで呼ぶ（`goToSelect` 経由でしか入らないため実際はnull不可能だが、防御が非対称）。
- `PlayerFactory::getPlayer()` は `return *m_player;` で、create前に呼ぶと未定義動作。現在の呼び出し順（InGame::spawnEntitiesでcreate直後）では安全だが保証がない。

**②深刻度**: 低

**③改善案**:
- **optional化は見送り推奨**。`std::optional<FadeTransition>` はFadeTransitionが参照メンバを持つためコピー/ムーブ代入が使えず、`emplace` 書き換えが必要になり差分の割に得るものが小さい。代わりにヌル安全の非対称だけ揃える（`FadingOut` ケースにも `if (m_fade)` を付けるか、状態遷移とfade生成を同時に行うprivateメソッドに集約）。
- assertは1行で追加:
```cpp
actor::Player& PlayerFactory::getPlayer() const
{
    assert(m_player && "PlayerFactory::getPlayer(): create()より前に呼ばれました");
    return *m_player;
}
```

**④判断**: assert=即実施。optional化=オーバーエンジニアリング寄りで見送り可。


---

## 9. リスト外の新規指摘（精査中に発見したバグ疑い・重複・品質問題）

マスターリストに載っていないが、今回のブランチで一緒に潰す価値が高いもの。

### 9-1. 【バグ疑い・高】`Select::notifyGameStart` のnullポインタ経路

```cpp
if (!m_windowManager || !m_windowManager->isJobSelected())
{
    m_windowManager->showWarningMessage("職業を選択してからスタートしてください。");
    // ↑ m_windowManager が nullptr でこの分岐に入った場合、null経由呼び出し
```
`!m_windowManager` が真のときに `m_windowManager->showWarningMessage` を呼ぶ。現在の生成経路（SceneFactoryが必ずsetWindowManagerする）では発火しないが、条件式が自らnullを想定している以上、内側もガードすべき:
```cpp
if (!m_windowManager) return;
if (!m_windowManager->isJobSelected())
{
    m_windowManager->showWarningMessage("職業を選択してからスタートしてください。");
    return;
}
startFadeOut();
```

### 9-2. 【バグ疑い・高】EntityId再利用×世代なし＝A3が「現役の危険」になっている

付録A3の再掲だが重要なので独立項目に。A2（Entity破棄）が実装されたことで、`EntityManager::m_recycledIds` による**IDの即時再利用が毎試合発生する**（弾は数秒で破棄→次の弾が同じIDを取る）。現在stale IDを保持し得る箇所:
- `ProjectileWindowManager::Slot::m_projectileId` — 破棄済み弾のIDをフェード中も保持。同フレームで同IDの新弾が出ると誤追従し得る。
- `AimComponent::m_targetId` — 死亡・破棄後のIDを次フレームまで保持。
- `InGame::m_macId` — ボスIDは破棄されない前提だが、保証はEnemyDeathSystemの実装依存。

改善はindex+generation方式（A3の通り）:
```cpp
// Entity.h
using EntityId = std::uint64_t; // 上位32bit=generation, 下位32bit=index
constexpr std::uint32_t entityIndex(EntityId id) { return static_cast<std::uint32_t>(id); }
constexpr std::uint32_t entityGeneration(EntityId id) { return static_cast<std::uint32_t>(id >> 32); }

// EntityManager
class EntityManager
{
public:
    Entity create()
    {
        std::uint32_t index{};
        if (!m_freeIndices.empty()) { index = m_freeIndices.back(); m_freeIndices.pop_back(); }
        else { index = static_cast<std::uint32_t>(m_generations.size()); m_generations.push_back(0); }
        return Entity{ (static_cast<EntityId>(m_generations[index]) << 32) | index };
    }
    void destroy(Entity e)
    {
        const auto idx{ entityIndex(e.getId()) };
        ++m_generations[idx];           // 旧IDを恒久的に無効化
        m_freeIndices.push_back(idx);
    }
    [[nodiscard]] bool isAlive(Entity e) const
    {
        const auto idx{ entityIndex(e.getId()) };
        return idx < m_generations.size() && m_generations[idx] == entityGeneration(e.getId());
    }
private:
    std::vector<std::uint32_t> m_generations{};
    std::vector<std::uint32_t> m_freeIndices{};
};
```
ComponentArrayのキーはEntityId(64bit)のままでよく、既存コードの変更は最小（EntityId型幅の変更とINITIAL_ENTITY_ID周りの整理）。導入後、`m_targetEntity.getId() != 0` のような生存チェックを `entityManager.isAlive(...)` へ順次置換できる。**今回のブランチの最優先候補**。

### 9-3. 【中】InGameのPlayerData二重初期化と初期化順の罠（A12関連）

- コンストラクタのメンバ初期化子で `PlayerData::fromMetadata(getMetadata(PLAYER).value())` を実行 → 直後の `loadResources()` で**再度**取得して上書き。1回目は「モデル未ロードでメタデータが無い場合」`optional::value()` が例外を投げ、loadResources内の丁寧なエラーログ・LOG出力に到達しない（エラーハンドリングが片方だけ機能する）。
- さらに `InGame.h` のメンバ宣言順（…m_fileEquipmentData → m_effectFactory → m_factoryManager → … → m_playerData → m_view）と初期化子リストの順（…m_playerData → m_effectFactory → m_view）が食い違っており、-Wreorder相当の警告対象。実初期化は宣言順なので現状バグではないが、読み手が初期化順を誤解する。

改善: m_playerDataの初期化子を空 `m_playerData{}` にして初期化はloadResources一本へ集約（loadModelById→getMetadataの正しい順序が1箇所になる）。初期化子リストは宣言順に並べ替える。あわせて `LOG(...)+throw` の直後にある `assert(playerMeta.has_value())` は到達不能（throwの後）なので削除（FactoryInitializer::initializeGroundにも同型のthrow後assertあり）。

### 9-4. 【中】重複コードの横断リスト（DRY違反）

| 重複内容 | 箇所 | 改善 |
|---|---|---|
| 敵種判定（AIマーカー→種別/表示名） | InGame.cpp / EnemyDeathSystem.cpp / DetectionAlertVisualsSystem.cpp | 2-1のEnemyTypeComponentへ一元化 |
| 拡張子→FileExtensionType解決＋小文字化 | FileEquipmentData::setFilePath / Win32SelectWindowManager.cpp | FileEquipmentData側へ一元化（2-1） |
| `DEG_TO_RAD` / `PI` の再定義 | MacAISystem.cpp / EnemyBehaviors.cpp / PlayerChargeVisualsSystem.cpp ほか | `core/utility/MathConstants.h` に `constexpr float PI / DEG_TO_RAD` を1箇所定義 |
| `SHAKE_Y_FREQUENCY_RATIO` 等シェイク定数と揺れ生成式 | DamageShakeSystem / MacAwakenEffectSystem | 揺れオフセット生成を小関数 `makeShakeOffset(phase, amplitude)` として共有（過度な共通化は不要、式1本の共有で十分） |
| 徘徊（pickWanderTarget・到着判定・pause） | MeleeChaseAISystem / RangeKeepAISystem | ロジックはほぼ同型。共通化するなら `PatrolBehavior` フリー関数群へ。ただし移動様式（地上/ホバー）が違うため、**pickWanderTargetの共有のみに留める**のが分けすぎないライン |
| EffectSystemの4イベントハンドラ（play→slot登録） | EffectSystem.cpp | `playAndTrack(entityId, type, position)` ヘルパ1本に集約（下記） |
| Camera位置計算（yaw/pitch/distance→pos） | CameraSystem::update / DebugCameraSystem::initializeFromOrbitCamera | `computeOrbitCameraPos(camera, transform)` として共有 |
| throw後の到達不能assert | InGame::loadResources / FactoryInitializer::initializeGround | assert削除（9-3） |

EffectSystemの集約例:
```cpp
void EffectSystem::playAndTrack(core::ecs::EntityId entityId,
    core::constant::EffectType type, const core::Vector3& position)
{
    const int handle{ m_effectFactory.play(type, position) };
    if (handle == -1) return;
    if (!m_componentManager.has<component::EffectComponent>(entityId)) return;
    m_componentManager.get<component::EffectComponent>(entityId)
        .m_slots.push_back({ type, handle });
}
```
4ハンドラは「位置の取り方」だけが違う3〜5行になる。

### 9-5. 【中】IComponent.h が自己完結していない

`core/ecs/IComponent.h` は `EntityId` を使うのに `Entity.h` をincludeしていない。現在はComponentArray.hのinclude順（Entity.h→IComponent.h）に救われてコンパイルできているだけで、IComponent.hを単独includeすると壊れる。`#include "Entity.h"` を1行追加。あわせてA23（実態はComponentArrayの型消去基底なので `IComponentArray` へ改名）も同時に実施すると1回の差分で済む。

### 9-6. 【低】FadeTransitionのAPI冗長

メンバに `m_uiRenderer` / `m_screen` を保持しつつ、`draw(uiRenderer, screen)` で同じものを引数でも受け取る二重化。メンバを使う形に統一して引数を削除（呼び出し7箇所の機械修正）。

### 9-7. 【低】Button::setOnClick がコピー格納（A15の残り）

```cpp
void Button::setOnClick(std::function<void()> callback)
{
    m_onClick = std::move(callback); // 現状: = callback（コピー）
}
```
同様に `Win32SelectWindowManager` コンストラクタの3コールバックも `std::move` 化。EventBusは5-1の改修に含まれる。

### 9-8. 【低】naming_convention違反の残り

- `LOG/LOG_W/LOG_E` マクロ（3-3で対応）。
- `game/event/SelectEvents.h` の `namespace core::event`（3-1で対応）。
- `ObjectPool.h` が `core/ecs/` 配置で `namespace core::base`（A24。ファイルを `core/base/` へ移すのが1手で済む）。
- インデント: `ServiceLocatorInitializer.h` や一部ファイルがスペースインデント、他はタブ。`.editorconfig` か `.clang-format` の導入で機械統一を推奨（リファクブランチの最後に整形コミットを1つ分けると差分レビューが楽）。

### 9-9. 【情報】ProjectileReflectSystemのTag書き換え設計

敵弾を反射時に `TagComponent` をPlayerへ書き換える方式は、AttackSystemの同陣営スキップとTargetingSystemの陣営判定に自然に乗る巧妙な実装。ただし「Tag＝所属」を実行時に書き換えることで、9-4の敵種判定のような「Tagで種別を推測するコード」を将来書いた人が壊れる。TagComponentのdoxygenコメントに「反射弾は所属が実行時に変わる」旨を1行追記しておくと事故を防げる。

---

## 付録A. ECS活用度・C++パフォーマンス24項目 — 現況確認（対応済 / 部分対応 / 未対応）

指示どおり再評価はせず、`src/` の現状との突き合わせのみ。

| # | 優先 | 要点 | 現況 | 確認根拠 |
|---|---|---|---|---|
| A1 | 🔴 | ComponentArray::get の暗黙生成 | **未対応** | `get()` が `return m_component[id];` のまま。`find`+assert / `tryGet` 未実装 |
| A2 | 🔴 | Entityが破棄されない | **対応済** | ProjectileSystem・EnemyDeathSystemが `removeAll`+`destroy` を実施。破棄予約→ループ末尾一括の形も実装済 |
| A3 | 🔴 | EntityIdに世代がない | **未対応（A2対応により危険度上昇）** | EntityManagerは `m_recycledIds` の単純再利用のまま。9-2参照 |
| A4 | 🔴 | publishのコピー＋ヒープ確保 | **未対応** | `std::any_cast<TEvent>(e)`（値キャスト）でリスナー毎に完全コピー |
| A5 | 🔴 | unsubscribe不在 | **未対応** | subscribeのみ。5-1参照 |
| A6 | 🔴 | 描画がECS迂回（RenderSystem不在・IDハードコード） | **部分対応** | InGameViewへ委譲・敵は動的リスト・弾はComponent走査に改善。player/groundは引数ID直指定のまま（2-3の汎用化案で完了にできる） |
| A7 | 🔴 | Input/Move/AnimationSystemが単一Entity専用 | **部分対応** | AnimationSystemは全走査化済。InputSystem/MoveSystemはplayerId固定・moveSpeed/dashMultiplierをSystemが保持したまま |
| A8 | 🟡 | getAllEntitiesの毎フレームvector確保・ループ内取得 | **未対応** | 全Systemで毎フレーム確保。AttackSystemは攻撃者ごとにresolveAttack内で取得（形を変えて残存、6-1参照） |
| A9 | 🟡 | unordered_map基盤・二重ハッシュ検索 | **未対応** | ストレージはunordered_mapのまま。ComponentManager::getComponentArrayもfind→operator[]の二重検索のまま |
| A10 | 🟡 | addの値渡し→コピー代入 | **未対応** | `add(EntityId, T component)` → `m_component[id] = component;` のまま |
| A11 | 🟡 | View drawの毎フレーム文字列連結＋SJIS変換 | **未対応** | LockscreenView::draw等で毎フレーム生成・変換。（なおSelectView自体は未使用ファイル化しており削除対象＝実質半減） |
| A12 | 🟡 | getMetadataの値返し／InGame二重初期化 | **未対応** | optional<ModelMetadata>値返し（内部にmap2個）のまま。二重初期化も残存（9-3） |
| A13 | 🟡 | CollisionSystemの約40行重複・O(n²) | **未対応** | Player vs Ground / Enemy vs Ground のブロックがほぼコピペのまま。VelocityComponent有無での統合案は依然有効（死亡バウンド分岐はDeathComponent有無で共通ブロック内に置ける） |
| A14 | 🟡 | Title::exitApp の std::exit(0) | **未対応** | `std::exit(0)` のまま（DxLib_End・ServiceLocator::clear・デストラクタ全スキップ）。Applicationの `m_isRunning` は既にあるので、SceneManager経由で終了要求を伝える（例: GameManagerに `requestQuit()` を足しApplicationがループ条件で見る）だけで直せる。PauseMenuのQuitは正しくm_isRunning経由なので、同じ経路に乗せるのが一貫 |
| A15 | 🟡 | std::functionのコピー格納 | **部分対応** | TitleView等は `std::move` 済。Button::setOnClick・Win32SelectWindowManagerコンストラクタ・EventBus::subscribeはコピーのまま（9-7） |
| A16 | 🟢 | InputManagerのunordered_map／const内挿入 | **未対応** | current/previousともunordered_mapのまま。`mutable`＋`operator[]` のconst内挿入も残存 |
| A17 | 🟢 | UIRenderer drawTextのpairキー構築＋複数回検索 | **未対応** | `make_pair(string, int)` を毎回構築、find後に `m_fontHandles[key]` で再検索 |
| A18 | 🟢 | EffectPoolの線形探索 | **未対応** | returnEffect/isActiveがm_activeSlots線形走査のまま（規模的に実害小） |
| A19 | 🟢 | GroundFactoryの使われないvector生成／EnemyFactoryの二重管理 | **未対応（削除で解決可）** | getAllGrounds等は未使用と判明（1-1で削除推奨）。EnemyFactoryのm_enemies＋m_enemyIds二重管理は残存 |
| A20 | 🟢 | AudioEventListenerのデッド=default | **未対応** | 1-2参照 |
| A21 | 🟢 | Cameraが引数無視の固定カメラ | **対応済** | Cameraは setLookAt/setFieldOfView の薄い層になり、計算はCameraSystemへ移管済 |
| A22 | 🟢 | Vector3の演算子不足 | **未対応** | operator*(float)/-=/lengthSq/normalize等が無く、手書き成分計算がMoveSystem/CameraSystem/AI系/View等に多数（`operator*` `length()` `normalized()` `dot()` を足すだけで数百行が短縮対象になる。全置換は無理せず、新規・修正ファイルから順次でよい） |
| A23 | 🟢 | IComponent→IComponentArray改名 | **未対応** | 名前そのまま（9-5のinclude修正と同時実施を推奨） |
| A24 | 🟢 | ObjectPoolの配置と名前空間のズレ | **未対応** | `core/ecs/ObjectPool.h` で `namespace core::base` のまま |

**集計**: 対応済 2（A2, A21）／部分対応 3（A6, A7, A15）／未対応 19。
🔴帯で残るのは A1 / A3 / A4 / A5 / A7(半分)。特に **A2だけ先行対応された結果、A3の危険度が旧レビュー時より上がっている**点に注意（9-2）。

---

## 付録B. 今回のブランチのチェックリスト（コピペ用）

```
優先度: 高
[ ] 9-2 / A3  EntityId世代化（index+generation）＋isAlive導入
[ ] A1        ComponentArray::get のassert化＋tryGet追加
[ ] 5-1/5-2 (A4/A5) EventBus: const参照渡し＋RAII購読トークン
[ ] 9-1       Select::notifyGameStart のnullガード修正
[ ] A14       Title::exitApp のstd::exit廃止（終了要求フラグ経由へ）

優先度: 中
[ ] 1-1       デッドコード一括削除（SelectView, Label, TitleのSplash系, drawBillboard,
              provideExisting, GroundFactory未使用API, UIManager::clear, KeyCode::R/Tab,
              AIComponent距離維持フィールド, PlayerDataパス系, IFactory, resourceManagerPtr,
              Selectの未使用ctor引数, Result::updateの未使用引数）
[ ] 1-2/A20   AudioEventListenerのデッド=default削除
[ ] 2-1/9-4   EnemyTypeComponent導入で敵種判定を一元化
[ ] 2-1/8-4   Select復元（restoreSelection）＋拡張子解決の一元化
[ ] A13       CollisionSystemのPlayer/Enemy vs Groundブロック統合
[ ] 7-5       攻撃クールダウンをAttackComponentへ一本化（AIComponent側削除）
[ ] 4-3       InGame::setupSystemsのリソース解決を関数抽出（buildTabProjectileSetup等）
[ ] 9-3/A12   PlayerData初期化の一本化＋初期化子リスト順の是正＋到達不能assert削除
[ ] 3-3       LogUtilをplatformへ移動・Windows.hのcpp隠蔽・LOGマクロ→core::log関数化
[ ] 6-1/A8    ComponentArray::forEach導入（Physics/Attack/AI系から適用）
[ ] 7-4       AttackStartEventにSeType追加（AudioEventListener購読追加）

優先度: 低（余力）
[ ] A22       Vector3演算子追加（* / -= lengthSq normalize dot）→順次置換
[ ] 9-4       PI/DEG_TO_RADのMathConstants集約、EffectSystem::playAndTrack集約、
              カメラ位置計算の共有
[ ] 8-2       MacAISystem stopMovementヘルパ（= {} 代入8箇所の解消）
[ ] 8-5       PlayerFactory::getPlayer にassert追加
[ ] 9-5/A23   IComponent→IComponentArray改名＋Entity.h include追加
[ ] A24       ObjectPool.h をcore/base/へ移動
[ ] 9-6       FadeTransition::drawの引数削除
[ ] 9-7/A15   setOnClick等のstd::move徹底
[ ] A11/A16/A17 文字列・キー・フォントキャッシュ系
[ ] 3-1       architecture.mdにコンポジションルート例外を明記 or IScreen拡張、
              SelectEvents.hの名前空間修正
[ ] 2-5       macDataへwindup/lockTime外出し、extensionBonus.json化

見送り（オーバーエンジニアリング判定）
[x] 4-1  インクルード集約ヘッダ → 作らない（PCHで代替）
[x] 4-4/8-3 DxLibラッパーヘッダ → 作らない（interface層が既に境界）
[x] 8-1  ISystemのComponentキャッシュ純粋仮想関数 → forEach＋tryGetで代替
[x] 8-5  fadeのoptional化 → unique_ptr維持（nullガードの対称化のみ）
[x] 7-1  シーンキャッシュ → 作り直し方式を維持と決定してTODO解消
[x] A9   sparse set化 → 現規模では効果薄、性能問題が出るまで保留
[x] 3-4/7-2 IViewProjection分離 → screenToWorldが必要になるまで保留
```

以上。

---
---

# 追補: 残り全ファイル精査の結果（第2パス）

初版レポート時点で未読・スキムだった以下を全て読了し、**これで src/ 全308ファイルの精査が完了**した:
infrastructure/repository 全実装（ModelRepository.cpp 412行含む）／infrastructure残り（AudioManager.cpp・EffectFactory.cpp・Screen.cpp・LogUtil.cpp・JsonKeys.h）／platform層全ファイル（WindowBase・WebView2Host・全Select系ウィンドウ・ProjectileWindow(Manager)・ResultWindow・LoadingWindow・WindowFactory・StringConverter・WindowsDataProvider・WindowsPerformanceProvider）／core/constant・core/data・game/constant・game/dataの全定義／ProjectileFactory・Ground／DebugHUDView・DebugGizmoView・PauseMenu系実装／resource.h。

以下、追補での新規発見。番号は初版セクション9の続き（9-10〜）。既存項目への追記は「1-1補遺」等で示す。

---

## 9-10.【高】難易度選択がゲームに未配線（DifficultyWindow）

**①現状**: `DifficultyWindow` はUI上でEasy/Normal/Hardを選択でき、Hard選択時は確認ダイアログまで実装済みで `m_selectedDifficulty` に保持する。しかし **`getSelectedDifficulty()` の呼び出し元がプロジェクト全体でゼロ**。`Win32SelectWindowManager` も `ISelectWindowManager` も難易度を外へ渡すAPIを持たず、GameManagerにも難易度の置き場がない。つまりプレイヤーが何を選んでもゲームプレイに一切反映されない。SelectViewの表示文言「難易度と武器、ファイルを３つ選択してください」とも不整合。

**②深刻度**: 高（機能の未完成。バグではなく「配線漏れ」）

**③改善案**: 職業選択と同じ経路をなぞるのが最小:
```cpp
// core/constant/Difficulty.h（新規）: enum class Difficulty { Easy, Normal, Hard };
// GameManager: DifficultySelectionData（または JobSelectionData に相乗り）を追加
// Win32SelectWindowManager コンストラクタに onDifficultySelect コールバックを追加し、
// DifficultyWindow::setOnDifficultyChanged(...) を新設して中継する
// InGame 側は敵HP/攻撃力係数などに反映（係数は enemyData.json の difficultyScale として持てば 2-5 と整合）
```
リリースまでに反映しない判断なら、DifficultyWindowごと非表示にする方が誠実（選べるのに効かないのはプレイヤーへの誤情報）。

**④判断**: 配線は適量。難易度ごとの完全な別パラメータセット（enemy JSONを難易度別に3セット）はオーバーエンジニアリング、係数方式で十分。

## 9-11.【中】jobData.jsonの二重ロードとSJIS往復変換（JobWindow / ParameterWindow）

**①現状**: `JobWindow::onCreateControls` が `m_jobRepository.emplace()`（`infrastructure::repository::JobRepository` を**直接生成**）しており、ResourceManagerが起動時にロード済みの `jobData.json` を**もう一度**読み込む。platform→infrastructureは依存方向として合法だが、同一データの二重ロード＋二重保持になっている。

さらにエンコーディングの往復がある: JobRepositoryはロード時に職業名を **UTF-8→SJIS変換して保持**（DxLib描画都合）→ `ParameterWindow::refresh` はそれをWebView(UTF-8)へ送るために**自前実装のsjisToUtf8ラムダで逆変換**している。`platform::utility::StringConverter` が存在するのに使わず、Win32 API呼び出しを再実装している点も重複。

**②深刻度**: 中（動作は正しいが、データフローが「UTF-8→SJIS→UTF-8」と一往復無駄で、変換実装が3系統ある）

**③改善案**: 根本は「**リポジトリはUTF-8のまま保持し、SJIS変換はDxLib描画の直前だけ**」に統一すること。SelectView/TitleView/PauseMenuViewは既にこの方式（描画時に `IStringConverter` で変換）なので、JobRepositoryから `utf8ToShiftJis` を外すだけで全体が一貫し、ParameterWindowの逆変換ラムダとJobWindowの `sjisToUtf8` 経路が丸ごと消える。二重ロードは、`ISelectWindowManager` 生成時に渡している `IResourceManager&` をJobWindowまで引き回して `getJobInfo` を使う（JobWindow内のJobRepository・include・optionalメンバを削除）。

**④判断**: 適量。削れるコード量の方が多い。

## 9-12.【中】拡張子→種別解決が3重実装（初版9-4の更新）

初版で2箇所（FileEquipmentData / Win32SelectWindowManager）と報告したが、第2パスで **`FileSelectWindow::openFileDialog` にも3つ目**の同一実装（rfind('.')→小文字化→Resolver呼び出し）を確認した。つまり同じ知識が **FileEquipmentData / Win32SelectWindowManager / FileSelectWindow の3箇所**。改善方針は初版9-4の通り（FileEquipmentDataへ一元化し、platform側はパス文字列を渡すだけ）で、対象が1箇所増えた。あわせて `FileSelectWindow` は自前で `m_filePaths`/`m_extensionTypes` を保持しており、Win32SelectWindowManagerの `m_slotPaths`/`m_slotExtTypes`、game側のFileEquipmentDataと合わせて**スロット状態が3箇所に複製**されている。真実の置き場をFileEquipmentDataの1箇所に決め、他は表示用キャッシュを持たない構造にするのが8-4（選択復元）と同時に解決できる。

## 9-13.【中】WebViewウィンドウのWndProc処理が5重コピペ

**①現状**: `WM_SIZE`（最小化ならsetVisible(false)、それ以外resize）／`WM_SHOWWINDOW`／`WM_ACTIVATEAPP`（クライアント再取得→resize）の3メッセージ処理が **JobWindow / FileSelectWindow / ParameterWindow / DifficultyWindow / ResultWindow で完全に同一のコードとして5回**書かれている（RulesWindowはWM_SIZE/WM_SHOWWINDOWのみの亜種、LoadingWindowは無し）。あわせて `onCreateControls` の「setIcon→setOnMessage→webView.initialize」も同型。

**②深刻度**: 中（挙動修正が5箇所同時修正になる。実際RulesWindowだけWM_ACTIVATEAPPが漏れており、既に差分が生まれている）

**③改善案**: WindowBaseとWebView2Hostの間に中間基底を1枚入れる:
```cpp
// platform/window/WebViewWindowBase.h（新規）
class WebViewWindowBase : public WindowBase
{
protected:
    using WindowBase::WindowBase;

    /** WebViewの標準メッセージ処理（サイズ追従・可視追従）。派生のonMessageから最初に呼ぶ */
    LRESULT handleWebViewMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) noexcept
    {
        handled = true;
        if (msg == WM_SIZE)
        {
            if (wParam == SIZE_MINIMIZED) m_webView.setVisible(false);
            else                          m_webView.resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        if (msg == WM_SHOWWINDOW) { m_webView.setVisible(wParam != 0); handled = false; return 0; }
        if (msg == WM_ACTIVATEAPP && wParam != 0)
        {
            RECT rc{}; GetClientRect(hwnd, &rc);
            if (rc.right > 0 && rc.bottom > 0) m_webView.resize(rc.right, rc.bottom);
            handled = false; return 0;
        }
        handled = false;
        return 0;
    }

    platform::webview::WebView2Host m_webView{};
};
```
各ウィンドウのonMessageは「handleWebViewMessagesを呼び、handledでなければ固有処理→WindowBase::onMessage」の3行になる。m_webViewメンバも基底へ集約され、6クラスから重複宣言が消える。

**④判断**: 適量（既に差分バグの芽が出ている実績があるため）。テンプレートメソッドパターンをさらに厳密化（onMessageをfinalにして仮想フックだけ公開等）するのはやりすぎ。

## 9-14.【中】resources.json を起動時に5回開いて5回パースしている

**①現状**: ModelRepository / FontRepository / ImageRepository / AnimationRepository / AudioRepository の各コンストラクタ・initializeが、**それぞれ独立に** `assets/config/resources.json` をopen→`nlohmann::json::parse`する。起動時のみとはいえ同一ファイルのフルパースが5回走り、ファイル欠落時のエラーメッセージも5種微妙に違う文言で分散している。

**②深刻度**: 中（起動時間・エラーハンドリングの一貫性）

**③改善案**: ResourceManagerのコンストラクタで1回だけパースし、各リポジトリへ `const nlohmann::json&` を渡す:
```cpp
// ResourceManager.cpp
std::ifstream file("assets/config/resources.json");
if (!file.is_open())
    throw std::runtime_error("assets/config/resources.json を開けませんでした");
const nlohmann::json shared{ nlohmann::json::parse(file) };
m_modelRepo = std::make_unique<repository::ModelRepository>(shared);
m_fontRepo  = std::make_unique<repository::FontRepository>(shared);
// ...（各リポジトリのコンストラクタ引数を const json& に変更）
```
jobData/stageData/projectileData/effectDataは元々別ファイルなので現状のままでよい。

**④判断**: 適量。「全JSONを1つのローダクラスに集約」まではやらなくてよい（リポジトリごとの責務分離は保つ）。

## 9-15.【中】ResourceManagerが初期化例外を握りつぶし、Fail Fast方針と矛盾

**①現状**: `ResourceManager` コンストラクタは全リポジトリ生成を `try { ... } catch (const std::exception& e) { LOG_E(...); }` で包み、**失敗してもアプリを続行**する。以降の全メソッドが `if (!m_xxxRepo)` の防御分岐を持つ羽目になっており、しかも getJobInfo / getStageMetadata / getProjectileMetadata は結局throwする（＝防御の一貫性もない）。GroundData.hのコメント「JSON読み込み失敗時はFail Fast」やリポジトリ各所のthrow運用と方針が真逆。

**②深刻度**: 中（リソース欠落時に「モデルが出ない・音が鳴らないまま起動する」中途半端な状態になり、原因究明が遅れる）

**③改善案**: catchを外して例外をApplication（コンポジションルート）まで伝播させ、そこでLOG＋メッセージボックス表示→終了にする。各getterのnullチェック分岐も全部消せる。開発中に一部リソース無しで起動したい事情があるなら、`#ifdef _DEBUG` で握りつぶす条件付きにして意図を明示する。

**④判断**: 適量（コード削減になる）。

## 9-16.【低】ModelRepository::parseJsonFile の必須キーが非対称＋JsonKeys.h完全未使用

- `j["collider"]["size"]` 等は **containsチェック無しで直アクセス**（欠落時はnlohmannの汎用例外で、どのファイルのどのキーかが分かりにくい）。transform/animations/gameplayは全てcontainsガード付きで非対称。colliderを必須とする意図なら、`if (!j.contains("collider")) throw std::runtime_error(filePath + ": collider がありません");` のような**ファイル名入りのエラー**にすると調査が速い。
- `infrastructure/constant/JsonKeys.h`（キー名定数集）は**参照ゼロの完全未使用ファイル**。parseJsonFileは全て生文字列を使っている。方針を決めて片方に寄せる: (a) JsonKeys.hを削除する（gameplayキーはgame側の `metadata_keys` と実質二重定義になっており、パース側で定数を使うなら本来そちらと共有すべきだが層をまたぐため難しい）、(b) 使うと決めてparseJsonFileを置換する。**推奨は(a)削除**。タイポはどのみち実データで即発覚する上、キー定数を2層で二重管理する現状が一番悪い。→ **1-1補遺（削除リスト追加）**
- gameplayキーの14連 `if (contains) floatProperties[...] = ...` は、キー名配列のループで7割短縮できる:
```cpp
static constexpr std::string_view FLOAT_KEYS[]{
    "moveSpeed", "dashMultiplier", "detectionRange", "attackRange", "maxHp", "defence",
    "attackPower", "attackCooldown", "attackWindup", "hoverHeight",
    "preferredDistanceMin", "preferredDistanceMax", "fireCooldown", "facingYawOffset" };
for (const auto key : FLOAT_KEYS)
    if (gp.contains(key)) metadata.floatProperties[std::string{key}] = gp[key];
```

## 9-17.【低】getMetadataの結果がloadModelByIdの呼び出し順に依存する（隠れた時間結合）

**①現状**: コライダー自動計算（size全成分0のとき）は `loadModelById` の中で行われ、**計算結果をm_metadataへ書き戻す**。そのため `getMetadata` を `loadModelById` より先に呼ぶと**自動計算前のsize=0が返る**。現在の呼び出し箇所（EnemySpawner・FactoryInitializer）は正しい順序だが、InGameコンストラクタの初期化子（初版9-3で指摘済み）は逆順で呼んでおり、まさにこの罠を踏んでいる（直後の上書きで実害が消えているだけ）。

**③改善案**: 9-3の修正（初期化子での取得をやめる）で実害は消えるが、構造的にはgetMetadataのdoxygenコメントに「colliderSize自動計算はloadModelById後に確定する」と1行明記するか、自動計算をコンストラクタのメタデータロード時（モデルを一時ロードして算出）へ移して時間結合自体を無くす。前者で十分。

## 9-18.【低】WebView2Hostの後始末の不整合

- `add_WebMessageReceived` は初期化直後に登録されるが、デストラクタの解除は `m_ready == true`（＝NavigationCompleted後）のときだけ。**ページ読み込み完了前に破棄すると解除されない**（コメントの「m_readyがtrueのときだけ登録している」は実装と不一致）。
- `m_navToken`（NavigationCompletedハンドラ）の解除が**どこにも無い**。
- WebView2の推奨手順である `m_controller->Close()` を呼んでいない。
- 非同期コールバックが生 `this` をキャプチャしており、初期化完了前にHostが破棄されるとuse-after-freeの可能性（現在の使い方ではウィンドウ寿命が長く実害は出にくい）。

**③改善案**: デストラクタを「`if (m_webview) { remove両トークン } if (m_controller) m_controller->Close();」に修正（条件をm_readyでなくポインタ有無に）。thisキャプチャ問題は、厳密にやるならweak参照化だが、**「WindowBase派生の寿命はWebView初期化完了より必ず長い」運用ならコメント明記で許容**（コールバック基盤の作り込みはオーバーエンジニアリング）。

## 9-19.【低】WindowBaseの細かい作法

- `SetWindowLongPtrW(hwnd, GWL_USERDATA, ...)` — 64bitでは `GWLP_USERDATA` が正式名。値は同じ(-21)だが、ファイル冒頭でわざわざ `#ifndef GWL_USERDATA constexpr int GWL_USERDATA{-21};` と自己定義しており、`GWLP_USERDATA` を使えばこの自己定義ごと不要。DesktopWindowは正しく `GWLP_USERDATA` ＋ `WM_NCCREATE` 方式で書かれており、**同一プロジェクト内で2流儀が併存**している。WindowBase側をDesktopWindow方式（WM_NCCREATEでUSERDATA設定）に揃えるのが正。
- `destroy()`: `DestroyWindow` 失敗時に `m_hwnd` をnullにしないまま `UnregisterClassW` を呼ぶため、失敗時に「クラスは消えたがhwndは残る」不整合になり得る（発生は稀）。
- `create()`: `RegisterClassW` が `ERROR_CLASS_ALREADY_EXISTS` で失敗すると即falseを返す。現在はProjectileWindowManagerがクラス名にインデックスを付けて回避しており動作は正しいが、`GetLastError() == ERROR_CLASS_ALREADY_EXISTS` を成功扱いにすれば呼び出し側の一意名生成という暗黙契約が不要になる。
- `ProjectileWindowManager::acquireFreeSlot` の `return &m_slots.back();` は、`m_slots.reserve(core::iface::MAX_PROJECTILE_WINDOWS)` をコンストラクタで行うと再確保由来のポインタ無効化懸念を根絶できる（現状も同一フレーム内使用のみで安全だが、防御として1行安い）。

## 9-20.【低】AudioManagerの細部

- `FADE_SPEED{0.01f}` は**1フレームあたり**の音量変化量で、update()がdeltaTimeを取らない＝フェード時間がフレームレート依存。固定タイムステップ（60fps）前提なら実害はないが、他の演出（FadeTransition等）が秒指定なのと非対称。`update(float deltaTime)` に変え `FADE_DURATION{1.67f}` 秒指定へ揃えると一貫する。
- 同じBGMを再生中に同じtypeで `playBgm` すると「別のBGM扱い」でフェードアウト→同曲フェードインが走る。`if (config.m_handle == m_currentBgmHandle && m_fadeState != FadeState::FadeOut) return;` の早期returnで無駄な曲の切れ目を防げる（Title↔Selectを行き来する場面では曲が違うため現状顕在化しないが、同曲シーンを増やすと露見する）。
- **`BgmType::Boss` が未再生**（typeMapとenumにあるが `playBgm(Boss)` の呼び出しゼロ）。ボス戦BGM切替（MacPhaseTransitionEventかDetection時に発火）が未配線。9-10の難易度と同じ「実装済みリソース・未配線」枠。仕様として入れないならenum値とtypeMapエントリを削除。

## 9-21.【低】LogUtilの追加事項（3-3の補強）

- リリースビルド（`_DEBUG` 無効）では全メソッドが空になるが、呼び出し側の `LOG(...)` マクロは **`std::format` を常に実行してから空関数に渡す**ため、リリースでもフォーマットコスト（文字列構築・ヒープ確保）が残る。3-3で提案した関数化の際、リリースでは早期returnかコンパイル時無効化（`if constexpr` ＋ビルドフラグ）を入れると完全にゼロコストにできる。
- `clear()` は全ビルドで空実装（ILoggerのインターフェースにあるが誰も呼ばない）。**1-1補遺: ILogger::clear / LogUtil::clear は削除候補**。

## 9-22.【情報】第2パスで品質が高かった箇所（変更不要の明示）

レビューは指摘だけでなく「触らなくてよい」の確定にも意味があるため記す: `WebView2Host` のpendingキュー（ready前送信の吸収）、`ProjectileWindow` のサイズ量子化・再描画抑制・入力無効化の作り込み、`DebugHUDView` の壁時計計測（固定タイムステップでFPSが正しく出ない理由までコメント化）、`WindowsPerformanceProvider` の差分方式CPU/ディスク計測、`EnemySpawner` のモデルハンドルプール、`AttackTelegraph` 系のAI/描画分離。これらは現状維持でよい。

---

## 1-1補遺: デッドコード削除リストへの追加（第2パスで確定した分）

初版1-1のリストに以下を**追加**する（いずれも宣言・実装以外の参照ゼロをgrepで確認済み）:

| # | 対象 | 備考 |
|---|---|---|
| a | `core/interface/IFileProvider.h`＋`platform/WindowsDataProvider.h/.cpp`＋ServiceLocator登録＋SceneFactoryの取得コード | `selectFile()` 呼び出しゼロ。ファイル選択はFileSelectWindowが自前ダイアログで実装済み。Selectの未使用ctor引数（初版1-1）と同根で、まとめて削除すると綺麗に消える |
| b | `infrastructure/constant/JsonKeys.h` | 参照ゼロ（9-16） |
| c | `SeType` の6値: `DeadPlayer` / `Skill` / `UiClick` / `UiHover` / `FileSelect` / `SceneTransition`（＋AudioRepositoryのtypeMap対応行） | 再生箇所ゼロ。UI系SEを今後実装する予定があるなら残す判断も可だが、現状はJSON設定してもコードが鳴らさない |
| d | `BgmType::Boss`（＋typeMap行） | 9-20。配線するか削除するかを決める（**配線推奨**。ボス戦の体験価値が高い） |
| e | `SelectWindowId::Stage` | bringToFrontのcaseにも存在しない未使用enum値 |
| f | `ISelectWindowManager::bringToFront`＋`Win32SelectWindowManager::bringToFront` | game/core側からの呼び出しゼロ（Desktopのタスクバー的UIから使う構想の名残と推測） |
| g | `core::constant::ui` の後方互換ピクセル定数（`FONT_SIZE_EXTRA_SMALL`〜`DEFAULT_FONT_SIZE` の7定数） | 使用ゼロ。コメント自身が「後方互換性のため」と認めている。なおMain.cppの「DxLibのマクロ(DEFAULT_FONT_SIZE等)と衝突」コメントの通り**衝突リスク源**でもあるため削除が安全 |
| h | `SceneFactory.cpp` の `MAIN_FONT_NAME` 定数 | 宣言のみで未使用。同名文字列がPauseMenuView.cppにハードコードされており、そちらは使用中→ResourceManager経由（`getFontName("main")`）へ寄せるとハードコード自体も消える |
| i | `ILogger::clear()`＋`LogUtil::clear()` | 呼び出しゼロ・全ビルド空実装（9-21） |
| j | `WindowBase.cpp` 冒頭の `GWL_USERDATA` 自己定義 | `GWLP_USERDATA` 使用に切り替えれば不要（9-19） |

---

## ファイル別 細かい指摘一覧（全ファイル対象・軽微含む網羅表）

初版・追補の本文で述べた主要指摘は参照番号で示し、本文未掲載の軽微な指摘はここに直接記す。**記載のないファイルは「指摘なし（現状維持でよい）」を意味する。**

### ルート直下
| ファイル | 指摘 |
|---|---|
| Main.cpp | DxLib include順の注意書きをdocs規約へ昇格（4-4）。指摘軽微 |
| Application.cpp | DxLib直呼び＝コンポジションルート例外の明文化 or IScreen拡張（3-1）。update内のDEBUGブロックが多くリリース削除タグは付いている（良） |
| ServiceLocatorInitializer.cpp | `resourceManagerPtr` 未使用（1-1）。IFileProvider登録は削除対象（1-1補遺a）。ヘッダがスペースインデント（9-8） |
| resource.h | 指摘なし |

### core/
| ファイル | 指摘 |
|---|---|
| base/ServiceLocator.h/.cpp | `provideExisting` 未使用（1-1）。それ以外は良好 |
| base/EventBus.h | A4/A5（5-1/5-2） |
| ecs/Entity.h | 世代なし（9-2/A3）。`INVALID_ENTITY_ID` と「未設定=0」判定の併存を世代化時に統一 |
| ecs/EntityManager.h | 9-2/A3 |
| ecs/ComponentArray.h | get暗黙生成（A1）、forEach/tryGet追加（6-1/8-1） |
| ecs/ComponentManager.h | 二重ハッシュ検索（A9）、add値渡し（A10） |
| ecs/IComponent.h | Entity.h include欠落＋改名（9-5/A23） |
| ecs/ObjectPool.h | 配置と名前空間のズレ（A24）。実装自体は良好 |
| ecs/SystemManager.h / ISystem.h | 指摘なし（8-1の仮想キャッシュ関数は追加**しない**判断） |
| utility/Vector3.h | 演算子不足（A22） |
| utility/Color.h | 指摘なし |
| constant/EffectType.h・SeType.h | 「typeMapにも追加を忘れずに」コメント＝文字列⇔enum対応の二重管理を自認。数が増えるならX-Macroかテーブル生成を検討（現数なら現状維持可）。SeType未使用6値（1-1補遺c） |
| constant/UI.h | 後方互換ピクセル定数7個削除（1-1補遺g） |
| constant/SelectWindowId.h | Stage未使用（1-1補遺e） |
| constant/JobType.h・BgmType.h | Boss未配線（9-20/1-1補遺d） |
| input/KeyCode.h | R/Tab未使用（1-1） |
| input/GamePadCode.h | パッド対応はisPadButtonDown等の実装のみでInputSystem側の分岐が薄い（機能として中途。仕様外なら現状維持可） |
| data/ModelMetadata.h | mapを2個内包→getMetadata値返しのコスト源（A12）。behaviors/animationsの新旧2形式併存はparseJsonFileコメントで明示済み（可） |
| data/その他（JobInfo/Mac/Projectile/Result/Stage） | 指摘なし。MacMetadata::phase()の設計（添字でなくenum参照）は良好 |
| event/IGameEvent.h・ISelectEvent.h | 指摘なし |
| interface/ILogger.h | マクロ＋ServiceLocator依存（3-3）、clear削除（1-1補遺i） |
| interface/IRenderer.h | drawBillboard未使用（1-1）、worldToScreen（3-4） |
| interface/IFileProvider.h | 丸ごと削除対象（1-1補遺a） |
| interface/その他 | 指摘なし |

### game/（初版で詳述済みのため差分のみ）
| ファイル | 指摘 |
|---|---|
| scene/SceneFactory.cpp | MAIN_FONT_NAME未使用（1-1補遺h）。他は初版7-1 |
| ui/pause/PauseMenuView.cpp | getLabelが毎フレーム文字列生成＋SJIS変換（A11同種）→ラベル3種をコンストラクタで変換キャッシュ。フォント名ハードコード（1-1補遺h） |
| ui/pause/PauseMenuController.cpp | 指摘なし（クリックエッジ検出の引き継ぎ処理は丁寧で良） |
| ui/debug/DebugHUDView.cpp | 指摘なし（9-22）。リリース削除タグ完備 |
| ui/debug/DebugGizmoView.cpp | 「浮遊型判定=RangeKeepAIComponentの有無」が2-1の敵種推測と同根→EnemyTypeComponent導入時に一緒に置換 |
| data/FileEquipmentData.h | setFilePathの拡張子解決を唯一の正とする（9-12）。getFilePath/getExtensionTypeが範囲チェック無し（hasSelectionはチェック有りで非対称）→同様のガード追加 |
| data/GroundData.h | getModelPath未使用の可能性（PlayerData同様、ロードはID経由）→確認の上削除候補 |

### infrastructure/
| ファイル | 指摘 |
|---|---|
| ResourceManager.cpp | 例外握りつぶし（9-15）、resources.json共有パース化の起点（9-14） |
| repository/ModelRepository.cpp | collider必須キー非対称＋ファイル名入りエラー化、gameplayキーのループ化（9-16）、getMetadata時間結合（9-17）。detachAllAnimationsのMAX_ATTACH_SLOTS{16}は妥当なワークアラウンド（コメント有・可） |
| repository/AudioRepository.cpp | 文字列→enumのtypeMapがSeType.hと二重管理（上記）。handle==-1時にcontinueで**無音のまま進む**→LOG_E追加推奨（Effect/Model系はログありで非対称） |
| repository/EffectRepository.cpp | 同上のtypeMap二重管理。handle==-1時ログなし（同上） |
| repository/JobRepository.cpp | SJIS変換をロード時に行う設計の見直し（9-11）。`json["jobs"]` 直アクセス（containsガード無し） |
| repository/Font/Image/Animation/Stage/Projectile | 9-14の共有パース化対象である以外は指摘なし |
| AudioManager.cpp | フレーム依存フェード・同曲再要求（9-20） |
| EffectFactory.cpp | デストラクタで `Effkseer_End()` 後にpools.clear()（スロットが持つプレイハンドルは既に無効化済みで実害なしだが、順序を「pools.clear→End」にする方が意味的に安全） |
| EffectPool.cpp | 線形探索（A18）。実装は堅実 |
| Renderer.cpp / UIRenderer.cpp | A17（フォントキー）以外は初版どおり |
| InputManager.cpp | A16。加えてKEY_MAPのR/Tabエントリ削除（1-1） |
| utility/LogUtil.h/.cpp | platform移設＋Windows.h隠蔽（3-3）、リリース時format空振り（9-21） |
| Camera/Screen/Animator | 指摘なし |

### platform/
| ファイル | 指摘 |
|---|---|
| window/WindowBase.cpp | GWLP_USERDATA/WM_NCCREATE方式へ統一、destroy失敗時の不整合、ERROR_CLASS_ALREADY_EXISTS許容（9-19） |
| window/select/Win32SelectWindowManager.cpp | 6ウィンドウ分のsetOnMinimize/Close・toggle分岐のテーブル化余地（構造は同型の5連if）。拡張子解決重複（9-12）。catch(...)握りつぶし2箇所→最低限LOG_Eを（デバッグ時に無言で機能不全になる）。復元API追加（8-4） |
| window/select/JobWindow.h/.cpp | JobRepository直接生成の除去（9-11）。WndProc共通化（9-13） |
| window/select/FileSelectWindow.cpp | 拡張子解決3重の1つ（9-12）、スロット状態の複製解消、WndProc共通化（9-13）。catch(...)→ログ |
| window/select/ParameterWindow.cpp | sjisToUtf8自前実装の除去（9-11）、WndProc共通化（9-13） |
| window/select/DifficultyWindow.cpp | **難易度未配線（9-10）**、WndProc共通化（9-13） |
| window/select/RulesWindow.cpp | WM_ACTIVATEAPP処理が他ウィンドウと非対称（9-13で基底に吸収すれば自動解消）。クラス名/タイトルがコンストラクタ直書き（他はstatic constexpr）→揃える |
| window/select/DesktopWindow.cpp | 指摘なし（USERDATA作法はむしろ手本） |
| window/result/ResultWindow.cpp | WndProc共通化（9-13）。pumpMessagesがWin32SelectWindowManagerと同一実装→共通化候補（WM_QUITのcontinue有無の差異あり、意図確認の上で） |
| window/loading/LoadingWindow.cpp | pumpMessagesが空実装なのにIWindowの契約上呼ばれ続ける（コメントで理由明示済み・可）。WndProc未処理（WebViewリサイズ無し）→9-13の基底導入で自動的に他と揃う |
| window/WindowFactory.cpp | createLoadingWindowとWin32SelectWindowManager::createAllWindowsで「DxLibクライアント矩形→スクリーン座標」計算が重複→小関数化候補（軽微） |
| window/projectile/ProjectileWindow.cpp | 指摘なし（9-22） |
| window/projectile/ProjectileWindowManager.cpp | slots.reserve追加（9-19）。stale ID懸念は9-2の世代化で解消 |
| webview/WebView2Host.cpp | トークン解除・Close（9-18） |
| utility/StringConverter.cpp | 指摘なし。ただしCP_ACP使用のためOSロケール依存（日本語Windows前提）。前提をコメント化推奨 |
| WindowsDataProvider.cpp | 丸ごと削除（1-1補遺a） |
| system/WindowsPerformanceProvider.cpp | 指摘なし（9-22）。DEBUGタグ完備 |

---

## チェックリスト追記（初版・付録Bへの差し込み）

```
優先度: 高（追加）
[ ] 9-10      難易度選択の配線（GameManager→InGameへ係数反映）または DifficultyWindow の一時撤去

優先度: 中（追加）
[ ] 9-11      JobRepositoryのUTF-8保持化＋JobWindowの二重ロード除去＋ParameterWindowの逆変換削除
[ ] 9-12      拡張子解決とスロット状態をFileEquipmentDataへ一元化（3箇所→1箇所）※8-4と同時
[ ] 9-13      WebViewWindowBase導入でWndProc 5重コピペを解消
[ ] 9-14      resources.jsonの共有パース化（5回→1回）
[ ] 9-15      ResourceManager初期化のFail Fast化（catch除去）
[ ] 9-20/1-1d BgmType::Bossの配線（フェーズ移行 or 発見イベントでplayBgm(Boss)）

優先度: 低（追加）
[ ] 9-16      parseJsonFileのcollider必須エラー改善＋gameplayキーのループ化＋JsonKeys.h削除
[ ] 9-17      getMetadataの時間結合をコメント明記（9-3修正とセット）
[ ] 9-18      WebView2Hostのトークン解除・Controller::Close追加
[ ] 9-19      WindowBaseのGWLP_USERDATA/WM_NCCREATE統一・destroy不整合修正・slots.reserve
[ ] 9-20      AudioManagerのdeltaTimeフェード化・同曲再要求ガード
[ ] 9-21      LOG関数化時にリリース早期return（3-3とセット）
[ ] 1-1補遺   a〜j の削除（IFileProvider一式 / JsonKeys.h / SeType6値 / SelectWindowId::Stage /
              bringToFront / UI.hピクセル定数 / SceneFactoryフォント定数 / ILogger::clear /
              GWL_USERDATA自己定義）
[ ] catch(...)握りつぶし箇所（Win32SelectWindowManager×2・各Window handleMessage）へのLOG_E追加
```

**精査完了宣言**: 本追補をもって `src/` 配下308ファイル・約22,900行すべてを読了した。interface群のうち純粋宣言のみのヘッダ（IAnimator/ICamera等の小ファイル）は実装側の読了により内容を確認済み。docs/ は評価規約の把握に必要な範囲（refactoring.md・architecture.md・naming_convention.md・ecs_performance_review.md・memo/todo）を参照した。

---

## 検収パス完了の追記（第3パス）

追補後の検収として、残っていた宣言のみの小ヘッダと切れていたファイル末尾をすべて読了した:
core/interface全13ヘッダ／game/system全ヘッダ（AI系含む）＋ChargeZoomSystem.cpp全文／残りシーンヘッダ（Lockscreen.h・Bios.h・Loading.h・Result.h・TitleView.h・LockscreenView.h）／Win32SelectWindowManager.h・ResultWindow.h/.cpp冒頭・WindowBase.h末尾・ModelRepository.h末尾／repository残ヘッダ5本（Image・Animation・Job・Stage・Projectile）／platform残ヘッダ7本（DesktopWindow・FileSelectWindow・DifficultyWindow・RulesWindow・StringConverter・WindowsPerformanceProvider・WindowFactory）／Renderer.h・UIRenderer.h全文／AudioManager.cpp・DebugGizmoView.cpp末尾。**これで未読領域はゼロ**。

検収パスでの新規発見は以下の1件のみ（他はすべて実装読了時の把握と一致し、新規問題なし）:

| # | 対象 | 備考 |
|---|---|---|
| k | `ResultWindow.cpp` 冒頭の `#include "core/base/ServiceLocator.h"` / `"game/scene/SceneManager.h"` / `"game/scene/SceneType.h"` の3本 | **いずれも未使用include**（本文で一切参照なし。シーン遷移を直接叩いていた時代の名残と推測。現在はonRetry/onTitleコールバック経由で正しく疎結合化されており、includeだけが残っている）。削除。1-1補遺リストに追加 |

また検収で確認できた良い点として、`WindowsPerformanceProvider.h` がHANDLEを `void*` で保持してWindows.hのヘッダ露出を防いでいる点は、3-3で指摘したLogUtil.hのWindows.h露出問題の**社内お手本**になる（LogUtil修正時はこの方式に倣うとよい）。

**最終宣言**: src/ 全308ファイル・約22,900行を全行読了し、refactoring.mdマスターリスト全項目の評価＋付録A 24項目の現況確認＋リスト外の新規指摘（9-1〜9-22＋1-1補遺a〜k）を完了した。　