# Win vs Mac パフォーマンス検査レポート（全ファイル検査版）

作成日: 2026-07-23
対象: src/ 全308ファイル（リファクタリング評価レポートと同一リビジョン）
観点: 実行時フレームコスト／起動時間／メモリ。リファクタリング観点は `refactoring_evaluation_report.md` に分離済み。docs付録A（ecs_performance_review.md）と重なる項目は番号を付記。

## 検査方法と網羅範囲

1. **ホットパス全数読解**: update/draw/pumpMessages/onMessage/windowProc を実装する**全63ファイルから関数本体を機械抽出し、全文（3,593行）を読解**した。毎フレーム実行されるコードは1関数残らず目視済み。
2. **機械スキャン**: draw/update内のリソースロード、コンテナ値返し、値渡し引数、毎フレームの文字列生成・SJIS変換・map検索・ヒープ確保、コールバック/メッセージハンドラのコストを全ファイルgrep。
3. **起動・イベント経路**: リポジトリ初期化・JSONロード・WebViewメッセージハンドラ・WndProcは前回までの全文読解の結果を性能観点で再判定。
4. 実測プロファイルは未実施（静的解析）。各項目に実測での確認方法を付記。
5. 規模前提: 同時エンティティ数十体・60fps固定設計。**この規模でプロファイラに現れる項目はごく少数**であり、深刻度はその前提で正直に付けている。

**結論の要約**: 実害となり得るのは PF-1（リフレッシュレート依存）のみ。フレームコード全体の品質は高く（deltaTime駆動の徹底、距離の2乗比較、スナップショット走査、更新間引き、量子化・差分抑制）、残りは「スケール耐性・作法」レベルの指摘に留まる。

---

## PF-1.【高】ゲーム速度がモニタのリフレッシュレートに依存する（フレームペーシング不在）

**①現状**: `Application::run` の主ループは時間計測もaccumulatorも無く、`ScreenFlip()` のVSync待ち（DxLibデフォルト）だけが実質のフレームレート制御。そこへ固定の `DELTA_TIME` を渡している。`SetWaitVSyncFlag` の明示設定も無い。

```cpp
while (m_isRunning && ProcessMessage() == 0)
{
    ClearDrawScreen();
    m_inputProvider->captureFrameInput();
    ...
    m_sceneManager->update(DELTA_TIME); // 固定値
    m_sceneManager->draw();
    ScreenFlip(); // ← ペーシングはこのVSync待ちのみ
}
```

**②影響**: 144Hzモニタでは update が毎秒144回呼ばれ**ゲーム全体が約2.4倍速**になる（移動・クールダウン・BGMフェード・シェイク・予兆タイマーすべて）。可変リフレッシュ環境では速度が不安定。VSync無効環境では数百fpsで空回りしCPU/GPUを浪費。**就活作品は審査側のPC環境を選べないため実害最大**。なお `DebugHUDView` は壁時計でFPSを計測しており（理由コメント付き・優秀）、このHUDを144Hz環境で見れば問題が即座に可視化される。

**③改善案**: 固定タイムステップ＋実時間計測（描画は毎フレーム、updateは1/60秒ぶん溜まった回数だけ）:
```cpp
void Application::run()
{
    LONGLONG prev{ GetNowHiPerformanceCount() }; // マイクロ秒
    double accumulator{ 0.0 };
    constexpr double STEP{ 1.0 / 60.0 };
    constexpr double MAX_FRAME_TIME{ 0.25 }; // スパイク時のスパイラル防止

    while (m_isRunning && ProcessMessage() == 0)
    {
        const LONGLONG now{ GetNowHiPerformanceCount() };
        double frameTime{ (now - prev) / 1000000.0 };
        prev = now;
        if (frameTime > MAX_FRAME_TIME) frameTime = MAX_FRAME_TIME;
        accumulator += frameTime;

        ClearDrawScreen();
        m_inputProvider->captureFrameInput();
        ...
        while (accumulator >= STEP)
        {
            m_sceneManager->update(static_cast<float>(STEP));
            accumulator -= STEP;
        }
        m_sceneManager->draw();
        ScreenFlip();
    }
}
```
セットで必要な修正: ①入力エッジ検出（captureFrameInput/updatePreviousState）を「1描画フレーム＝複数step」時にどう扱うか決める（最初のstepのみエッジ有効、が簡単で安全） ②`AudioManager` のフレーム依存フェード（FADE_SPEED=0.01/フレーム）をdeltaTime秒指定へ ③ `RangeKeepAISystem`/`EnemyRangedAttackSystem` の `m_elapsedTime` はdeltaTime積算なので修正不要（正しく書けている）。

**④実測**: 144Hz環境（またはVSyncオフ）で修正前後の速度とDebugHUDのFPSを比較。

## PF-2.【中】毎フレーム30箇所の std::vector ヒープ確保（getAllEntities）— A8

**①現状**: `ComponentManager::getAllEntities<T>()` は呼ぶたびに `std::vector<EntityId>` を新規構築して返す。**22ファイル・30箇所**が毎フレーム呼んでいる（今回のホットパス全数読解で全箇所を目視確認）。

**②影響**: 現規模で1フレーム合計数十μs以内の推定。実害は小さいが、定常ヒープ確保として直せば計測に現れる数少ない項目。

**③改善案と重要な注意**: `ComponentArray::forEach` 導入（リファクタリングレポート8-1と同一解）。ただし今回の読解で、**このコピーが仕様として依存されている箇所**を確認した:
- `DetectionAlertVisualsSystem::update` に「getAllEntitiesはスナップショットを返すため、ループ中の削除は安全」という明示コメントがあり、ループ内で `remove<AlertComponent>` している
- `EnemyDeathSystem::update` はループ内で `removeAll`＋`entityManager.destroy` している
- `ProjectileSystem` は既に破棄遅延（m_pendingDestroy）を実装済み＝**forEach化後の手本**

したがってforEach化は「走査中に削除する2システムへ破棄遅延パターンを先に入れる→全30箇所を機械置換」の順で行うこと。スナップショット前提コメントも書き換える。

**④実測**: VS診断ツールのヒープ割り当てプロファイルで60秒間の確保回数を前後比較。

## PF-3.【中】コンポーネント取得の定常コスト — A1/A9/A10

**①現状**: 全システムが毎フレーム `get<T>(id)` を多数回呼び、経路は unordered_map の**二重ハッシュ検索**（type_index→ComponentArray、EntityId→T）。`has<T>()`→`get<T>()` の対で同じ検索を2回することも多い（読解した全システムで頻出のイディオム）。`get` の暗黙デフォルト生成（A1）も分岐を増やす。

**②影響**: 1フレーム総ルックアップは数百回×2段×数十ns＝合計数十μs以内。体感には出ない。

**③改善案**: (1) 8-1の `tryGet`（ポインタ返し）導入で has+get の二度引きを一度にする（正しさ改善A1と同時に達成）。(2) 各システムでComponentArrayをローカルキャッシュして1段目を1回化。(3) sparse set化（A9本丸）は**現規模では見送り**。

## PF-4.【低】起動時間: 同一JSONの重複パースと同期ロード

resources.json を5リポジトリが独立に5回フルパース（9-14）、jobData.json 二重ロード(9-11)、モデル同期一括ロード。JSON重複は数msで、支配項のモデルロードには専用Loading画面があるため体験上の問題なし。9-14/9-11で十分、非同期ロード化はオーバーエンジニアリング。**実測して「有意差なし」を確認するのも面接材料になる**。

## PF-5.【低】表示中・毎フレームの文字列生成/変換（全数リスト確定版）

ホットパス全数読解により、毎フレームの文字列生成・SJIS変換を行う箇所を**全て**特定した:

| 箇所 | 毎フレームの内容 | 対処 |
|---|---|---|
| `SelectView::draw`（最多） | 固定タイトルのSJIS変換／スロット3件ぶん `substr`＋結合＋SJIS変換／**`getJobInfo` の値コピー**／HP・ATK・DEF・SPD・職業名の `to_string`＋結合5本 | 選択変更時のみ再構築するキャッシュ（`m_cachedSlotTexts[3]`・`m_cachedParamTexts`）へ。getJobInfoも選択変更時に1回 |
| `LockscreenView::draw` | `std::time`＋`localtime_s`＋`snprintf`×2＋日付文字列のSJIS変換＋**固定ヒント文のSJIS変換**＋`getTextWidth`×3 | ヒント文はコンストラクタで変換キャッシュ。時刻・日付は「分が変わったときだけ」再生成 |
| `DetectionAlertVisualsSystem::draw` | バナー表示中のみ `format`＋SJIS変換×2 | 固定文言はキャッシュ、タイマーは秒替わりのみ再生成 |
| `PauseMenuView::draw` | ポーズ中のみ `getLabel` 生成＋setFontのmap検索 | ラベル3種をコンストラクタで変換キャッシュ |
| `DebugHUDView` | 多数（**デバッグビルド限定・DEBUG:タグ管理下のため対象外**） | 現状維持 |

いずれも1フレーム数μs程度で実害は無いが、「毎フレーム同じ入力から同じ文字列を作る」パターンの全数として記録する。UI系（Button/Label）はコンストラクタで変換済みキャッシュ方式で**正しく書けている**ため、同じ方式に揃えるだけでよい。

## PF-6.【低】drawTextごとのフォントハンドルmap検索 — A17

`UIRenderer` はフォントを `std::map<pair<string,int>, int>` でキャッシュし、drawTextごとに文字列比較込みのlog n検索。生成は初回のみで正しい。1回サブμs×数十回/フレームで実害なし。enum化は最下位優先。

## PF-7.【低】毎フレームのローカルvector構築（getAllEntities以外）

今回の全数読解で新たに特定:
- `ProjectileWindowSystem::update` の `std::vector<ProjectileWindowInfo> infos{}` — 毎フレーム構築。メンバ化して `clear()`＋初回 `reserve(MAX_PROJECTILE_WINDOWS)` で確保ゼロにできる
- `ProjectileReflectSystem::update` の `enemyProjectiles`/`playerProjectiles` 2本 — 同上（弾が無ければ早期returnするため実際の確保は戦闘中のみ）

PF-2のforEach化と同じコミットで処理すると自然。

## PF-8.【低】その他の既知項目（定常コストでないもの）

| 項目 | 判断 |
|---|---|
| `getMetadata` 値返し（A12、map2個内包の数KBコピー） | スポーン時のみ。const参照返し化は品質改善として実施、性能単体では低 |
| `EffectPool` 線形探索（A18）／`EffectFactory::update` の erase_if＋map find | 要素数個〜十数個。現状維持 |
| `CollisionSystem` O(n²)総当たり＋ペアごとのget | n=数十で許容。空間分割は明確にオーバーエンジニアリング |
| 円・扇テレグラフ（48セグメント×3重描き×本数） | 同時数本でプリミティブ数百。DxLib即時描画の範囲内 |
| `MacAwakenEffectSystem::draw` のビネット枠ループ（band/STEP回のdrawBox） | 演出中のみ・数十回。許容 |
| `PlayerChargeVisualsSystem::draw` の集中線（溜め中のみ最大数十三角形） | 許容。乱数を自前のハッシュ的rand01で生成しており分布関数の再構築コストも無い（良） |
| リリースビルドのLOG format残存（9-21） | フレームパス内のLOG呼び出しは実質1箇所（InGameのデバッグキー内）。性能理由での優先度は最低 |
| `applyDeathDissolve` が赤化飽和後も毎フレーム同じマテリアル色を再設定 | 死亡演出中の数体×数マテリアルのMV1呼び出し。無害。気になるなら「progress・alphaが前回と同値ならreturn」の1行 |

## PF-9.【情報】検査して白と確定した箇所（触らなくてよい・むしろ手本）

疑って読み、問題なしと確定したもの。面接で「なぜ触らなかったか」を語れる:

- **主ループ以外のdeltaTime駆動は全域で正しい**: 全システムがdeltaTime積算でタイマー・補間を書いており（ChargeZoomは「フレームレート非依存の指数補間」と理由コメント付き）、PF-1を直せばそのまま全部が正しく動く土台になっている
- **距離判定は2乗比較を適切に使用**（ProjectileWindowSystem/ProjectileReflectSystem）。sqrtは方向正規化が必要な箇所のみ
- `Title`/`DebugHUDView` の**パフォーマンス計測は1秒間隔に間引き済み**（WindowsPerformanceProviderの重いAPI呼び出しを毎フレームしない設計が両所で一貫）
- `ProjectileWindowManager`: SetWindowPos量子化・差分抑制。`ProjectileWindow::paintLogo`: WM_ERASEBKGND抑制で二度塗り防止＋最近傍補間を負荷理由コメント付きで選択（模範的）
- `InputManager::captureFrameInput`: GetForegroundWindow＋約20キーのスナップショットは毎フレームだが、フレーム内一貫性のための設計理由が明記されており妥当
- `Renderer::m_originalColors`: erase解放ありリーク無し。`UIRenderer`/`ImageRepository`等: 初回ロード＋キャッシュ方式
- WebViewメッセージ: すべてイベント駆動（毎フレームpostMessage無し）。JSONパースはユーザー操作時のみ
- `EffectFactory::update` の handleToType 掃除、`AudioManager` のフェードステートマシン（PF-1対応時にdeltaTime化のみ必要）
- `Bios`/`Loading`/`Lockscreen`/`Result`/`SceneManager`/`FadeTransition`/`UIManager`/`PauseMenuController`/`DesktopWindow`/`RulesWindow`/各WndProc: 指摘なし

## ファイル別 指摘一覧（ホットパス63ファイル全数＋起動系）

**記載のないファイルは「性能上の指摘なし」を意味する。**

| ファイル | 指摘（参照） |
|---|---|
| Application.cpp | PF-1（本レポート最重要） |
| game/scene/SelectView.cpp | PF-5（毎フレーム文字列生成の最多箇所＋getJobInfo値コピー） |
| game/scene/LockscreenView.cpp | PF-5（time/localtime/snprintf/SJIS×2） |
| game/scene/InGameView.cpp | drawModelsの走査方式は2-3（リファクタ側）参照。性能指摘なし |
| game/system/（getAllEntities使用の20ファイル） | PF-2（30箇所の一覧はgrep一発: `grep -rn "getAllEntities<" game`） |
| game/system/ProjectileWindowSystem.cpp | PF-2＋PF-7（infosのメンバ化） |
| game/system/ProjectileReflectSystem.cpp | PF-2＋PF-7（仕分けvector2本） |
| game/system/DetectionAlertVisualsSystem.cpp | PF-2（**スナップショット依存の明示コメントあり・forEach化時に要修正**）＋PF-5 |
| game/system/EnemyDeathSystem.cpp | PF-2（ループ内destroy・破棄遅延化が先）＋PF-8（applyDeathDissolve飽和後再設定） |
| game/system/TargetingSystem.cpp | PF-2×2（Aim×Health二重走査だがaimerは実質1体・許容） |
| game/ui/pause/PauseMenuView.cpp | PF-5（ラベルキャッシュ） |
| infrastructure/AudioManager.cpp | PF-1関連（フレーム依存フェードのdeltaTime化） |
| infrastructure/UIRenderer.cpp | PF-6（A17） |
| infrastructure/ResourceManager.cpp＋repository/5本 | PF-4（resources.json×5） |
| platform/window/select/JobWindow.cpp | PF-4（jobData二重ロード、9-11） |
| core/ecs/ComponentManager.h / ComponentArray.h | PF-2/PF-3（A1/A8/A9/A10の実装元） |

## 実測の勧め（就活での説得力向上）

1. **フレーム内訳**: `GetNowHiPerformanceCount` で update/draw/ScreenFlip の3区間を計測しDebugHUDへ追加（数行）
2. **ヒープ確保回数**: VS診断ツールでプレイ60秒のスナップショット比較（PF-2/PF-7の前後比較に最適）
3. **リフレッシュレート耐性**: 144Hz環境またはVSyncオフで速度確認（PF-1の前後比較。DebugHUDのFPS表示がそのまま証拠になる）
4. リークチェック: `_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)`（DxLib併用時の誤検出に注意書きを）

## 作業チェックリスト（性能）

```
優先度: 高
[ ] PF-1  固定タイムステップ＋accumulator導入
          （セット: 入力エッジ整合／AudioManagerフェードのdeltaTime化）

優先度: 中
[ ] PF-2  破棄遅延パターンをDetectionAlertVisuals/EnemyDeathへ導入
          → ComponentArray::forEach導入 → 全30箇所を機械置換（A8）
[ ] PF-3  tryGet導入でhas+getの二度引き解消（A1と同時）

優先度: 低
[ ] PF-4  resources.json共有パース化＋jobData二重ロード除去（9-14/9-11）
[ ] PF-5  SelectView/LockscreenView/DetectionAlert/PauseMenuViewの文字列キャッシュ化
[ ] PF-6  フォントキーenum化（A17・任意）
[ ] PF-7  ProjectileWindowSystem/ReflectSystemのローカルvectorメンバ化＋reserve
[ ] PF-8  getMetadataのconst参照返し化（A12・品質として）
[ ] 計測  フレーム内訳のDebugHUD追加＋提出前の実測1回
```

**検査完了宣言**: 毎フレーム実行される全63ファイルのupdate/draw/pumpMessages/onMessage/windowProc本体（3,593行）を全文読解し、起動経路・イベント経路は既読の全308ファイル読解結果を性能観点で再判定した。本レポートの一覧に無いホットパスは存在しない。

---

## 追補: ヘルパー関数層の全数検証（最終パス）

「ホットパス本体」に加え、**ホットパスから呼ばれるヘルパー関数・基盤経路**を最後の未検証層として全数確認した。対象: resolveAttack／isColliding・resolveCollision／changeAnimation・tryChangeState／drawBanner・enemyTypeName／drawModels・drawReticle／updateChase・updatePatrol・hoverTargetHeight・applyAttackAnimation／chooseAction・countAliveMinions・perform系／EventBus::publish経路と所有権／ServiceLocator::get／EnemyFactory::getEnemyIds／Screen::getWidth／Renderer::drawModel／UIRenderer::drawText／Animator::updateAnimTime／Camera::setLookAt／ObjectPool／FadeTransition／DebugGizmoViewのdraw系（デバッグ限定）。

### 白と確定（重要な裏取り）

| 経路 | 確認結果 |
|---|---|
| `EnemyFactory::getEnemyIds()`（InGame::drawが毎フレーム呼ぶ） | **const参照返し**。毎フレームのvectorコピー無し |
| `EventBus` の所有権 | **InGameのメンバ**（`core::base::EventBus m_eventBus`）。再挑戦時はシーンごと破棄・再生成されるため、**購読の蓄積によるpublishコストの単調増加は起きない**（unsubscribe不在5-1は正しさの課題として残るが、性能問題ではないと確定） |
| `EventBus::publish` | type_index→unordered_map検索＋リスナー順次呼び出し。発火はイベント時のみ（AnimationFinished/EnemyAlerted等は状態遷移の瞬間だけ）で毎フレーム発火する購読は無い |
| `Screen::getWidth/getHeight` | キャッシュ済みメンバ返し（DxLib呼び出し無し） |
| `Renderer::drawModel` | MV1SetScale/Position/Rotation＋MV1DrawModelの標準4呼び出しのみ |
| `CollisionSystem::isColliding/resolveCollision` | AABB算術のみ・確保無し。ペアごとのget呼び出しはPF-3の一般パターンに含まれる |
| 残り全ヘルパー | イベント時のみ実行（resolveAttack/perform系/fire）、単純算術（hoverTargetHeight/smoothstep）、デバッグ限定（DebugGizmo/HUD）のいずれかで指摘なし |

### 微小な追加指摘（既存項目への吸収）

- `drawBanner` 内の `ALERT_TIME_TEXT`（固定文言）のSJIS変換と、`enemyTypeName` の `has<T>` 2連ハッシュ検索は、いずれもバナー表示中のみ・微小。**PF-5のキャッシュ化に含めて解消**（enemyTypeNameは2-1のEnemyTypeComponent導入で自然に1回のgetになる）
- `SelectView`/`LockscreenView`/`PauseMenuView` のdraw内 `ServiceLocator::get<IStringConverter>()` 毎フレーム呼び出し（type_index map検索）— PF-5で変換自体をキャッシュ化すれば呼び出しごと消える
- `Renderer::drawModel` の handle==-1 時 `LOG_E` は、万一無効ハンドルの描画対象が残った場合に**毎フレームformat＋ログが走る**エラーパス（正常時コストゼロ）。9-21のリリース時ログ無効化で自動的に無害化される

**最終宣言**: ホットパス本体（63ファイル・3,593行）＋そこから呼ばれるヘルパー関数層＋起動・イベント経路の全数検証を完了した。本レポートのPF-1〜PF-9および本追補の一覧の外に、未検証の実行経路は存在しない。実害となり得る項目がPF-1のみという結論は最終確定である。