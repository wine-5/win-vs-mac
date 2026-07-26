# Win vs Mac 総合レビュー完全版（パフォーマンス＋設計＋実装手順）

作成日: 2026-07-25
構成: 第Ⅰ部 パフォーマンス検査（PF-A〜M） / 第Ⅱ部 アーキテクチャ・設計レビュー（AR-1〜28） / 第Ⅲ部 リファクタリング実装手順書（STEP 1〜5）

---

# 第Ⅰ部

# Win vs Mac パフォーマンス検査レポート v2

作成日: 2026-07-25
対象: src/ 全313ファイル（.cpp 106 / .h 207、約24,500行）
観点: 実行時フレームコスト・シーン遷移/起動時間・メモリ。静的解析ベース（全ファイルの読解＋呼び出し箇所の全数grep）。

前回レポートの最重要項目だったフレームペーシング不在（PF-1）は、`Application::run` の accumulator＋`MAX_UPDATES_PER_FRAME` 方式で修正済みであることを確認した。ポーズ中のaccumulator破棄、処理落ち時の切り捨て、入力の前回状態をupdate回数と独立させる設計まで含めて正しく実装されている。本レポートは今回のリビジョンで新たに、または引き続き残っている項目のみを扱う。

なお前提として、このコードベースは既に相当よく最適化されている。CollisionSystemのメンババッファ再利用、ProjectileWindowのサイズ量子化による再描画抑制、EffectPool、フォントハンドルキャッシュ、モデルハンドルキャッシュ＋MV1DuplicateModel、LightSystemの光源生成の初回限定化など、重い処理を避ける設計判断がコメント付きで随所にある。以下の指摘の多くは「現在の規模（エンティティ百のオーダー）ではプロファイラに現れないレベル」であり、深刻度はその前提で付けている。

---

## PF-A.【高】InGameシーン構築の同期ロードがメインスレッドを止める（遷移時のフリーズ・応答なしリスク）

現状、`SceneManager::changeScene(SceneType::InGame)` の中で `InGame` のコンストラクタが同期的に走り、その中で `FactoryInitializer` 経由の `MV1LoadModel`・`LoadSoundMem`・コライダー自動計算（`MV1SetupReferenceMesh` による全頂点走査）までがすべてメインスレッド上で完了するまでブロックする。この間 `ProcessMessage()` が呼ばれないため、ロードが数秒かかるステージではDxLib側のゲームウィンドウがWindowsに「応答なし」と判定されうる。ローディング演出はWebView2の別ウィンドウ（描画は別プロセス）なのでアニメーション自体は動き続ける可能性が高いが、ゲームウィンドウ本体は白くなったりドラッグ不能になったりする。コードベース全体に `std::thread` / `std::async` / `SetUseASyncLoadFlag` は一切存在しないことをgrepで確認した。

対処は2案ある。手軽なのはDxLibの `SetUseASyncLoadFlag(TRUE)` でロードを非同期化し、Loadingシーンのupdateで `GetASyncLoadNum()` が0になるのを待ってからInGameへ遷移する方式（ただしロード完了前のハンドル使用に注意が必要で、初期化順の再設計が要る）。より単純な代替は、InGameの構築を数フレームに分割してフレームごとに1〜2アセットずつ読み、各フレームで必ず `ProcessMessage`＋`ScreenFlip` に戻る方式。応答なしの回避だけならこちらで十分。

実測での確認: リリースビルドで重いフォルダを選んでLoading→InGame遷移中にゲームウィンドウをドラッグしてみる。固まればこの問題。

## PF-B.【中〜高】WebView2ウィンドウごとに独立した環境生成（メモリとセレクト画面遷移の遅延）

`WebView2Host::initialize` は呼ばれるたびに `CreateCoreWebView2EnvironmentWithOptions` を実行する。WebViewWindowBase を継承するウィンドウは FileSelect / Parameter / Difficulty / Rules / Result / Loading の6種あり、特にセレクト画面では `Win32SelectWindowManager::createAllWindows` が4枚を一斉に生成する。環境パラメータが同一ならブラウザプロセスは共有されるが、コントローラ1つにつきレンダラプロセスが1つ付くため、セレクト画面表示中はWebView2関連だけで数百MB規模のワーキングセットになりうる。また環境生成→コントローラ生成の非同期チェーンがウィンドウごとに走るため、4枚の表示完了までの遅延も積み上がる。

対処: `ICoreWebView2Environment` をアプリで1つだけ生成して共有し、各ウィンドウは `CreateCoreWebView2Controller` だけを行う構成にする。加えて、ユーザーデータフォルダを明示指定する（現状nullptrなので、exeが書き込み不可の場所に置かれると環境生成自体が失敗する。パフォーマンスではなく堅牢性の問題だが同じ箇所なので併記）。シーン離脱時にコントローラを確実にCloseしているかも確認しておくとよい。

実測での確認: セレクト画面表示中にタスクマネージャで msedgewebview2.exe のプロセス数とメモリ合計を見る。

## PF-C.【中】getAllEntities が毎フレーム新規vectorを確保する（37箇所＋Debug HUD）

`ComponentArray::getAllEntities` は呼ばれるたびに `std::vector<EntityId>` を確保して返す。呼び出しは26システム・ビューにまたがり37箇所、さらにDebugHUDが統計表示のために毎フレーム2回、InGameViewが敵数カウントで1回呼ぶ。60fpsなら毎秒2,400回前後のヒープ確保＋unordered_map全走査になる。ヘッダコメントにある通り現規模では許容範囲で、意図的なトレードオフとして文書化もされている。ただし直すなら方向は2つ: (1) `forEach(callback)` 形式の走査APIを足して確保ゼロで回す（外部インターフェースの追加だけで済み、コメントに書かれた「内部差し替え可能」の思想とも整合する）、(2) CollisionSystem::collectBoxes と同様に各システムがメンバvectorを持って `clear()`＋再利用する。就活の面接では「なぜ直していないか」を規模根拠で語れる状態が既にできているので、優先度は低くてよい。

## PF-D.【中】Component アクセスごとの二重ハッシュ検索（型検索＋Entity検索）

`ComponentManager::get/has/tryGet` は毎回 `typeid(T)` → `type_index` 構築 → `m_componentArrays` のハッシュ検索で配列を特定し、その後 `ComponentArray` 内でEntityIdのハッシュ検索を行う。つまり1アクセスで2回のハッシュ検索が必ず発生する。find→operator[]の二重検索を1回に統合した改善は既に入っているが、型検索側は残っている。ホットなシステム（AttackSystem, GroundingSystem, InGameView::drawModels など）はエンティティ1体あたり5〜8回アクセスするため、ここが全フレームコストの中で最も広く薄く効いている。

対処: 各システムがコンストラクタで `ComponentArray<T>&` を直接受け取って保持する（型検索がゼロになる）か、`getComponentArray<T>` 内で `static ComponentArray<T>* cached` のような型ごとのローカルキャッシュを使う。前者はDIの設計とも合う。

## PF-E.【中】UVスクロールモデルの描画で毎フレーム冗長なDxLib呼び出し

`InGameView::drawModels` は `needsUv` が立つモデルについて毎フレーム `setTextureScroll` を呼び、その中で全テクスチャの `MV1SetTextureAddressMode`（WRAP設定）と全フレームの `MV1SetFrameTextureAddressTransform` を実行する。問題は2点ある。第一にアドレスモードはモデルにつき一度設定すれば変わらないため、毎フレームの再設定は完全に冗長。第二に `needsUv` の条件は「uvScaleが1でない or スクロールオフセットが0でない」なので、タイリングだけしてスクロールしない配置物（uvScale≠1・速度0）も毎フレーム変換を掛け直される。ステージの床・壁がタイリング前提で大量にある場合、ここがDxLib呼び出し数を不必要に押し上げる。

対処: アドレスモード設定はモデルロード時（`loadModelByPath` 内、既に `disableBackCulling` を掛けている場所）へ移す。フレーム変換は RenderComponent に「前回適用したoffset/scale」を持たせ、変化したときだけ呼ぶ。スクロール中のモデルは毎フレーム変わるので呼び続けることになるが、静的タイリングの分が丸ごと消える。

## PF-F.【低〜中】リリースビルドにデバッグ機構が常設される構造

`DebugGizmoView` / `DebugHUDView` / `DebugCameraSystem` は `#ifdef _DEBUG` ではなく「リリース時に削除すること」というコメント運用で管理されており、現状は無条件に生成・毎フレーム実行される。DebugHUDだけでも毎フレーム getAllEntities×2、PerformanceDataProvider、DrawStringToHandle 7行分のコストがある。Gizmoの球・カプセル描画は分割数を落とす配慮が入っているが、そもそもリリースで動くべきでない。手作業削除は消し忘れリスクがあるため、生成箇所とdraw呼び出しを `#ifdef _DEBUG` で囲むのが安全。`InGame` コンストラクタのマウスカーソル表示（「リリース時にfalseへ」コメント）も同じ構造の問題なので同時に対処するとよい。

## PF-G.【低】Application::run の update内ループで毎回ServiceLocator検索

固定タイムステップの内側ループが毎update `ServiceLocator::get<IAudioManager>()` を呼んでいる。type_index構築＋ハッシュ検索が毎秒60回（処理落ち時は最大300回）。コンストラクタで一度取得してメンバに保持すれば消える。実害は微小だが、最もホットな場所なので直す価値はある。

## PF-H.【低】UIRenderer::drawText のキー構築と二重検索

呼び出しごとに `std::pair<std::string,int>` のキーを構築（フォント名のstringコピー。長い名前ならヒープ確保）し、`std::map`（順序付き・文字列比較）で `find` した後に `operator[]` で再検索している。BIOSシーンのように1フレームで多数の行を描く場面では文字列比較×2が行数分積まれる。`std::unordered_map`＋カスタムハッシュに替え、`find` の結果イテレータをそのまま使う（`getTextWidth` は既にそうしている）だけで半減する。

## PF-I.【低】GroundingSystem / CollisionSystem のペアごと三角関数

CollisionSystem::toGroundLocal は rider×ground の全ペアで `sin/cos` を計算するが、groundのyawはフレーム内で不変なので `collectBoxes` の時点でcos/sinを事前計算してBoxに持たせられる。GroundingSystem も riders×surfaces のペアごとに `surfaceHeightAt` で回転計算をしており、同様にsurface側を毎フレーム冒頭で一度だけ変換しておける。また GroundingSystem は surface のめぼしい候補を絞らず全面を見るので、面数が増えるステージ（Cドライブ級）では riders×surfaces がそのまま効く。XZのAABBで事前に足切りするだけでも大半をスキップできる。

## PF-J.【低】LightSystem の消失Entity検出が線形探索

`destroyLightsOfLostEntities` はライトごとに `std::find` で aliveEntities（vector）を線形走査する。ライト数×エンティティ数の比較になるが、現状ライトは少数なので実害なし。ライトを増やす予定があるなら aliveEntities を `unordered_set` にするだけでよい。

## PF-K.【情報】設計上のコストとして把握しておくべき点（対処不要）

弾の実OSウィンドウは1発ごとに毎フレーム `SetWindowPos`＋`AdjustWindowRectEx`＋`GetWindowLongPtr`×2 が走る。サイズ量子化・SHOWWINDOWの条件化・TOPMOSTの拡張スタイル化と、再描画を抑える工夫は既に尽くされており、残るコストはこのギミックの本質的な代金。MAX_PROJECTILE_WINDOWS の上限があるので暴走もしない。`Renderer::m_originalColors`（ディゾルブの元色キャッシュ）は消費側でeraseされておりリークしないことも確認済み。EffectPoolのactiveSlots線形探索はエフェクト同時数が少ない前提で問題ない。

## PF-L.【中】攻撃予兆の円・扇が1つあたり約150回の個別3D描画呼び出し

`Renderer::drawGroundCircle / drawGroundSector` は48セグメントの `DrawTriangle3D` / `DrawLine3D` を1本ずつ呼び、TelegraphVisualsSystem は予兆1つにつき下地・進行・リングの3レイヤーを重ねるため、予兆1個で約150回のDxLib描画呼び出しが発生する。Macのノヴァ攻撃や召喚した雑魚が同時に予兆を出す場面では、これだけでDrawCall数が数百〜千のオーダーで跳ねる。さらにセグメントごとに `std::cos/std::sin` を毎フレーム計算し直している（前フレームと同じ角度なのに）。

対処: DxLibの `DrawPolygon3D`（VERTEX3D配列＋頂点数を1回で渡す）に置き換えれば、予兆1レイヤーが1呼び出しになる。sin/cosは `constexpr` 不可でも、48分割の単位円テーブルを初期化時に1度作って使い回せば毎フレームの三角関数96回が消える（半径・中心はテーブル値への乗算・加算で適用できる）。DebugHUDにDrawCall数表示が既にあるので、予兆多発時の数値をビフォーアフターで比較すると効果が定量化できる。

## PF-M.【微小】ロック画面・タイトルの毎フレーム文字列生成

LockscreenView::draw は毎フレーム、日付文字列に加えて固定文言「クリックまたはキーを押してください」まで `std::string` を構築して `utf8ToShiftJis` 変換に掛けている。固定文言の変換結果はコンストラクタで1度作ってメンバに持てば消える。日付も分が変わったときだけ再生成すれば十分（`tm_min` を前回値と比較）。TitleView も毎フレーム `std::to_string`＋文字列結合でパーセント表示を作っている。いずれも該当シーン限定かつ文字列数個なので実害はほぼゼロだが、「毎フレーム不変値を再計算しない」という原則の例として直しやすい。

---

## 優先順位のまとめ

体感に直結するのは PF-A（遷移フリーズ）と PF-B（WebView2のメモリ・遅延）の2つで、ここだけは実測の上で対応を勧める。PF-L（予兆のDrawCall）はボス戦の予兆多発時にだけ効く局所的な山なので、Mac戦でフレーム時間を実測してから判断するのがよい。PF-C〜E はフレーム時間の「広く薄い」部分で、面接で設計判断として語れるように現状の根拠（規模前提）を持っておけば、直すのは処理落ちが実測されてからで遅くない。PF-F はパフォーマンスというよりリリース品質の保険として早めに `#ifdef` 化しておくのが安全。PF-G/H は5分で直せる系。

実測する場合は、DebugHUDに既にFPS・フレーム時間・DrawCall数・CPU/メモリの表示基盤があるので、120Hz/144Hzモニタ環境と、ファイル数の多いフォルダ（Hard相当）を選んだ状態の2条件で数値を取るのが効率的。

---

# 第Ⅱ部

# Win vs Mac アーキテクチャ・設計レビュー

作成日: 2026-07-25
対象: src/ 全323ファイル（約26,200行）
観点: レイヤー依存・責務分離・クラス設計・一貫性・定数の重複。パフォーマンスは対象外（別レポート参照）。

先に全体評価を述べる。レイヤー構造は本物である。includeの全数チェックで、core は game/infrastructure/platform を一切参照せず、game は infrastructure/platform を一切参照せず、infrastructure は game を一切参照しない。抽象（core/interface）を介した依存性逆転が徹底されており、配線はコンポジションルート（ServiceLocatorInitializer）に集約されている。EventBusのRAII Subscription、IDamageHandlerのChain of Responsibility、ComponentArrayの「内部実装を差し替え可能」なカプセル化とトレードオフの文書化など、設計判断が言語化されている点も強い。以下は、その水準を前提とした上での指摘である。深刻度は【高】＝レイヤー違反・ドメインロジックの漏出、【中】＝肥大化・一貫性の乱れ、【低】＝改善余地、で付けた。

---

# 第1部 レイヤー・責務の違反

## AR-1.【高】platform層にゲームのドメインロジックが漏出している（唯一のレイヤー違反）

includeの全数チェックで検出された逆流はこの1系統だけである。`Win32SelectWindowManager.cpp` が `game/constant/ModelId.h`・`game/constant/MetadataKeys.h` を、`Win32SelectWindowManager.h` と `FileSelectWindow.h` が `game/utility/FileExtensionTypeResolver.h` をincludeしている。

問題はinclude行そのものより中身にある。`updateParameterWindow()` は playerData のメタデータから基礎HP・攻撃・防御・速度を読み出し、装備スロットごとの拡張子ボーナスを合算して最終ステータスを計算している。これは「ファイルを装備するとステータスが上がる」というこのゲームの中核ルールそのものであり、ウィンドウを生成・配置するプラットフォーム層が持つべき知識ではない。このままだと、ステータス計算式を変えるときにWin32のウィンドウ管理クラスを触ることになる。

さらに全数走査で決定的な事実が判明した。**game層には `PlayerData::applyExtensionBonus`（atk/spd/def/hpの加算）が既に存在する**。つまりこれは「ロジックが間違った層にある」だけでなく、「正しい層に既にある計算を、platform層がもう一度書いている」二重実装である。ボーナスの計算式（現在は単純加算）を将来乗算や上限付きに変えたとき、インゲームの実効値とセレクト画面のプレビュー値が食い違う事故が構造的に保証されてしまっている。

対処: game層に `EquipmentPreviewCalculator`（スロットのパス一覧を受け取り、基礎値＋ボーナスの計算結果DTOを返す純粋なクラス）を切り出し、Win32SelectWindowManager はそのDTOを `ParameterWindow::refresh` に流すだけにする。計算機はコールバックまたはcore/interfaceの小さな抽象として注入すればincludeの逆流も消える。拡張子→タイプ解決（FileExtensionTypeResolver）も同様にgame側で解決してから結果だけを渡す。副次効果として、ステータス計算が単体テスト可能になる。

## AR-2.【高】core/data がゲーム固有のドメイン知識を抱えている

core/data には `FileExtensionBonus`（hp/atk/def/spd）、`MacMetadata`（ボスのフェーズ・技の重み・覚醒条件）、`ProjectileMetadata`（ダメージ）、`StageMetadata` が置かれている。coreは「どのゲームでも使い回せる基盤層」のはずだが、「Macというボスに覚醒フェーズがある」という知識は本作固有そのものである。こうなった経緯は明確で、これらを返す `IResourceManager` が core/interface にあるため、戻り値の型もcoreに置かざるを得なかった構図である。

対処は2段階ある。本格的にやるなら IResourceManager を「汎用アセットロード（モデル・画像・音・フォント）」と「ゲームデータリポジトリ（ステージ・敵・弾・ボーナス）」に分割し、後者のインターフェースと型を game/data へ移す。infrastructureの実装クラスが両方を実装すれば配線は変わらない。現実解としては、core/data を「レイヤー間で共有する契約DTOの置き場」と再定義してその方針をコメントで明文化する手もあるが、MacMetadata のような固有名詞級の型が居る時点で説明が苦しいので、少なくともMac関連だけでもgame側へ動かすことを勧める。

## AR-3.【中】InGameシーンにゲーム進行ルールが埋没している（635行の主因）

InGame.cpp はプロジェクト最大の635行だが、リソースロード・エンティティ生成・約30システムの登録という「シーンのコンポジションルート」としての部分は正当な大きさで、問題ではない。問題は `setupEvents()` のイベント購読ラムダの中に、撃破カウント、被ダメージ集計、「開始時の雑魚が全滅したらボス出現」、「ボスが消滅したら勝利リザルトへ」「プレイヤー死亡演出完了で敗北リザルトへ」という勝敗・進行のルールが匿名関数として埋まっていることである。ゲームの勝利条件という最重要仕様が、シーンクラスのプライベートラムダの中にしか存在しない。

対処: `StageProgressionSystem`（または GameFlowDirector）を切り出し、m_stageEnemyIds・m_killCount・m_macId・m_totalDamageTaken とイベント購読ごと移す。勝敗確定は `GameOverEvent{isVictory}` のようなイベントで発行し、InGame はそれを受けてリザルト保存と遷移だけを行う。これで InGame は配線専任に戻り、勝利条件が独立したクラス名で読めるようになり、進行ルールの単体テストも可能になる。

## AR-4.【中】シーンがServiceLocator経由でSceneManagerを呼ぶ循環構造

SceneManager → SceneFactory → 各Scene という所有方向に対し、各Sceneは `ServiceLocator::get<SceneManager>()` で親を呼び戻して遷移している（6箇所）。循環参照をロケータが隠蔽している形で、シーン単体を SceneManager なしでテストできない。対処: `ISceneNavigator`（changeScene 1メソッドの狭いインターフェース）を切ってシーンにコンストラクタ注入するか、シーンのupdateが遷移要求を戻り値（`std::optional<SceneType>`）で返して SceneManager 側が処理する方式にする。後者は循環が完全に消えるのでより綺麗。

## AR-5.【中】ファットインターフェース: IRenderer と IResourceManager

IRenderer は18メソッドあり、汎用モデル描画、ドメイン演出（applyDeathDissolve＝敵死亡の赤化、drawSpinningModelFacing＝レインボー弾専用）、デバッグ描画（drawDebugSphere/Capsule/Collider）、計測（getDrawCallCount）が同居している。利用側の大半は1〜2メソッドしか使わないのにインターフェース全体に依存する（ISP違反）。特にデバッグ描画3種はDebugGizmoViewしか使わないので `IDebugRenderer` へ分離すると、リリースビルドからデバッグ機構を消す作業（パフォーマンスレポートPF-F）とも噛み合う。applyDeathDissolve は「死亡演出」という利用文脈がインターフェース名に漏れているので、汎用の「マテリアル色ブレンド＋アルファ」APIとして言い換えるとcoreに置く正当性が立つ。

IResourceManager も同様に、アセットロードとメタデータ提供に加えて `computeBoundingRadius/Center/Size`・`detachAllAnimations` というモデル計算ユーティリティが混ざっている。これらはリソースの「管理」ではなくモデルの「操作」なので、IAnimator か新設の小さなモデルユーティリティへ移すと責務名と中身が一致する。

## AR-6.【低〜中】GameManagerが用途の異なる状態の寄せ集めになっている

GameManager は終了要求フラグ、デバッグモードフラグ、連続ジャンプ（チート）フラグ、ファイル装備データ、リザルトデータを1クラスで持つ。それぞれ寿命も利用者も異なる（quit はApplication、デバッグフラグは開発時のみ、装備とリザルトはセッションをまたぐゲームデータ）。小型ながらデータハブ化の初期症状で、今後「とりあえずGameManagerに足す」が起きやすい。装備＋リザルトを `GameSession` として分離し、デバッグフラグ群は `#ifdef` 化と合わせて `DebugSettings` へ、と3分割すると各クラスが1行で説明できるようになる。

---

# 第2部 クラス設計と一貫性

## AR-7.【中】文字列変換（IStringConverter）の取得方法が場当たり的

ほぼ全システム・ビューがコンストラクタ注入で統一されている中、日本語文字列のShift-JIS変換だけは Button / PauseMenuView / ObjectiveView / LockscreenView が描画のたびに `ServiceLocator::get<IStringConverter>()` を直引きし、DetectionAlertVisualsSystem に至っては同じロケータ取得をメンバへ遅延キャッシュするという第3の方式を取っている。DIポリシーの例外が5箇所、しかも3通りのやり方で存在する状態である。

根治策はコンバータの存在自体を消すことで、`UIRenderer::drawText` がUTF-8を受け取って内部で変換（またはDxLibのSetUseCharCodeFormat設定）すれば、呼び出し側から変換の概念ごと消える。文字コードは描画基盤の関心事であってUIロジックの関心事ではないので、責務の置き場としてもそちらが正しい。

## AR-8.【中】InGameViewの11本のsetterによる2段階初期化

InGameView は draw系システム6種＋デバッグビュー2種＋HUDビュー3種を、生成後に個別型のsetterで受け取る。設定し忘れてもコンパイルは通り、nullチェックで黙って描画がスキップされるだけなので、結線漏れが実行時にも検知されない。また新しい描画物を足すたびに「setter追加→InGameで呼ぶ→drawに1行足す」の3点セットが必要で、変更が3ファイルに散る。

描画順の決定権をViewに置く方針自体は正当（ヘッダに意図が明記されており、半透明やHUDの重なり順は集中管理すべき）なので、受け渡しだけを一般化するとよい。`IDrawable`＋描画レイヤーenum（World3D / WorldOverlay / Screen / HUD / Debug）を定義し、`addDrawable(layer, drawable)` で登録、Viewはレイヤー順に回すだけにする。setterは消え、追加時の変更は登録1行になる。ISystemにdrawが無いのに6システムがdrawを持つ非対称も、IDrawableを別インターフェースとして明示することで「updateするもの」と「描くもの」の直交した設計として説明できる形になる。

## AR-9.【低】actorディレクトリの命名が実態とずれている

game/actor の Player / EnemyBase は、実体はエンティティを組み立ててComponentを付与する一回きりのセットアップクラスであり、ランタイムに存在し続ける「アクター」ではない。ECSに移行した歴史の痕跡と思われるが、初見の読者（面接官を含む）は「ECSなのにActorクラスがある？二重構造？」と誤解する。factory/ 配下へ移して PlayerBuilder / EnemySetup 等へ改名するだけで誤解が消える。AnimationSetup は既にその命名になっているので、揃える方向は明確。

## AR-10.【低】MacAISystem（474行）は境界事例

update内のステートマシン、4種の技（melee/ranged/nova/summon）の実行、重み付き抽選、覚醒フェーズ管理を1クラスで持つ。技ごとに `IBossAction` 戦略へ分解する余地はあるが、ボス1体の専用ロジックとして関連が強く凝集しているとも言え、分割すると却って読みにくくなるリスクもある。現状維持でよいが、面接で「なぜ分割しないのか」と聞かれたときに「技同士がフェーズデータと windup 状態を共有しており、分割コストが凝集の利益を上回ると判断した」と答えられるように意図を持っておくこと。技を今後も増やす予定があるなら、その時が戦略パターン化のタイミングである。

## AR-11.【低】EnemyDataのstringly-typedなメタデータ読み出し

EnemyData は `metadata_keys` の文字列キーで `floatProperties`（map<string,float>）から14個の値を1つずつ引いて型付きゲッターに詰め替えている。キーの集中管理（MetadataKeys.h）とデータクラスへの詰め替えという2段の防御は正しく機能しており、生文字列の散在は防げている。残る弱点はキーのtypoや欠落が実行時まで分からないことなので、詰め替え時に必須キーの欠落をログではなくロード失敗として扱う（fail-fast）ようにしておくと、JSONを編集する将来の自分への保険になる。構造自体の変更は不要。

---

# 第3部 定数の重複（既存の定数クラスを使っていない箇所）

ご指名のあった観点なので機械的に全数抽出した。「複数のcppで同名・同値のconstexprが定義されている」パターンと「定数クラスに実体があるのに直書きしている」パターンの両方を挙げる。

## AR-12.【要修正】円周率の再定義4箇所（しかも精度が劣る）

`core/utility/MathConstants.h` に `std::numbers` ベースの PI / TWO_PI / DEG_TO_RAD が既にあるにもかかわらず、PlayerHUDView.cpp・ObjectiveView.cpp・HudPanel.cpp・InGameView.cpp の4箇所が `3.141593f` / `6.283185f` を手打ちで再定義している。手打ち値はstd::numbersの丸め値と微妙に異なるため、厳密には「同じπ」ですらない。includeを1行足して消せる、最も費用対効果の高い修正。

## AR-13.【要修正】BASE_SCREEN_HEIGHT{1080} が4ファイルで重複

PlayerHUDView / ObjectiveView / EquipmentSlotView / HudPanel がそれぞれ基準解像度1080を定義している。core/constant/UI.h の FONT_SIZE_*_RATIO は既にコメントで「28 / 1080」のように1080基準を前提としており、基準値との結合が事実として存在するのに定数だけが散っている。UI.h に `BASE_SCREEN_HEIGHT` を1つ置き、4ファイルから参照する。

## AR-14.【要修正】手動同期を明言している物理定数の重複

`DEATH_BOUNCE_RESTITUTION{0.5f}`・`DEATH_BOUNCE_MIN_SPEED{20.0f}` が CollisionSystem と GroundingSystem の両方に定義され、GroundingSystem側のコメントに「CollisionSystemと揃える」と手動同期が明記されている。片方だけ調整して挙動が縦（着地）と横（衝突）で食い違う事故が構造的に起こりうる。game/constant に `DeathPhysics.h` のような小さな共有ヘッダを作って一本化する。同型の重複として `GROUND_LIFT{2.0f}`（TelegraphVisualsSystem / AttackTelegraphVisualsSystem — 予兆の浮かせ量が2システムでずれると地面へのめり込みが片方だけ起きる）、`ANIM_FALLBACK_TIMEOUT{5.0f}`（EnemyDeathSystem / PlayerDeathSystem）も同じ対処が要る。

## AR-15.【中】HudPanelという共通部品があるのにパネル定数が各Viewで重複

`PANEL_MARGIN{28}`・`PANEL_PADDING{20}` が PlayerHUDView と ObjectiveView の両方に定義されている。HudPanel クラスが共通部品として存在するのだから、余白定数はHudPanel（またはUI.h）が持ち、各Viewは参照するだけにすべきである。HUDの余白を調整するときに2箇所直す運用は、片方の直し忘れで揃わなくなる。DebugHUDView と PlayerHUDView の `LABEL_Y{16}` も同種。

## AR-16.【中】画面比マジックナンバーの直書き（UI.hのRATIO方式を使っていない）

InGameView のレティクル描画は `base * 0.030f`（リング半径）、`0.018f`（ティック長）、`0.006f`（隙間）、`0.0028f`（ドット半径）と画面高さ比の値を直書きしている。UI.h に FONT_SIZE_*_RATIO という「画面比は名前付き定数にする」方式が確立しているのだから、RETICLE_RING_RATIO 等として同じ場所か無名namespaceの名前付き定数に揃える。値の意図（なぜ0.030なのか）が名前で残るのが本質的な利益。

## AR-17.【低】色の直書き23箇所

`Color::rgb(255, 0, 0)` のような即値指定が23箇所ある。Color.h には RED / YELLOW どころか WINDOWS_LOGO_BLUE や TELEGRAPH_* まで名前付き色を整備する文化があるのだから、頻出の rgb(255,0,0)（デバッグの攻撃範囲）や rgb(200,0,0)（覚醒ビネット）、rgb(255,48,48)（レティクル捕捉色）は名前を付けて寄せる。特にレティクル捕捉色とビネット赤のような「演出上の意味を持つ赤」は、名前がないと後から同じ赤系を使うべき場面で値がばらける。デバッグ描画の色は許容範囲だが、リリースコードの演出色は揃えたい。

## AR-18.【低】その他の小さな重複

Renderer.cpp 内で drawGroundCircle と drawGroundSector がそれぞれ `SEGMENTS{48}` を定義している（同一ファイル内なので無名namespaceへ1つにまとめるだけ）。TitleView の `INFO_PANEL_PADDING{14}` はAR-15のパネル余白群と統合するか検討。

---

# 第4部 全数走査（323ファイル読了）による追加所見

初回レビュー後に全クラスの本文を読了して見つかった追加項目。データ専用のComponentヘッダ33本はメソッド保有の機械検査で全数確認し、ロジック混入がないこと（request()等の軽量ヘルパ2件のみ）を確認済みである。

## AR-19.【中】敵種別のハードコードが、プロジェクト自身のデータ駆動方針と矛盾している

EnemyBase のクラスコメントは「敵種ごとの派生クラスは廃止し、生成に必要な差分はすべてEnemyData（JSON由来）が持つ」と明確に宣言しており、behaviors/animationsのレシピ方式は実際その通り実装されている。ところが `EnemyDeathSystem::isFallingDeath` だけは「Safariなら落下死、Xcode/Macならアニメ死」を `EnemyType::Safari` のenum比較でコードに直書きしている。新しい浮遊敵を追加するとJSONだけでは完結せず、このシステムの修正が必要になる（OCP違反であり、自ら立てた設計方針への自己矛盾でもある）。EnemyData に deathStyle（"fall" / "animate"）を1項目足してJSONへ移せば、宣言と実装が一致する。敵種enumの直書き分岐はgrepの結果この1箇所だけだったので、直せば「差分はすべてデータ」が完全に成立する。

## AR-20.【中】徘徊ステートマシンが2つのAIシステムでコピペ実装されている

`MeleeChaseAISystem::updatePatrol` と `RangeKeepAISystem::updatePatrol` は「pauseタイマー消化 → 目的地が無ければ抽選 → 目的地へ移動 → 到着したら停止してpause抽選」という同一の骨格を、共有の PatrolComponent に対してそれぞれ実装している。差分は本質的に「RangeKeep側はホバー高度の維持が挟まる」ことと調整値だけである。徘徊の仕様変更（例：目的地を選び直す条件の追加）は現状2箇所の同期修正になる。共通の `updatePatrolCore(patrol, transform, 調整値, 移動適用コールバック)` を切り出せば、各システムには移動の適用方法（地上／ホバー）だけが残る。データ（PatrolComponent）は共有済みなのにロジックだけ二重、という中途半端な状態が惜しい。

## AR-21.【中】PhysicsSystemのジャンプエッジ検出がEntityごとでなくシステム単位

`m_prevJumpPressed` はシステムのメンバ変数1つだが、全Entityを回すループの中でエッジ検出（押した瞬間の判定）に使われている。現在InputComponentを持つのはプレイヤー1体だけなので正しく動くが、入力駆動のEntityが2体になった瞬間（協力プレイ、リプレイ再生、AIへの入力注入など）、互いの前回状態を上書きし合って壊れる。「前フレームの入力状態」はEntityの状態なのだから InputComponent 側に持たせるべきで、実際 PlayerAttackComboSystem も同型の `m_wasAttackPressed` をシステムメンバで持っており同じ構造的リスクがある。動いているうちに直すのが安い典型例。

## AR-22.【低】ResultWindowの二重インターフェースをファクトリが隠し、dynamic_castで復元している

ResultWindow は `IWindow` と `IResultWindowManager` の両方を実装しているのに、`IWindowFactory::createResultWindow` は `unique_ptr<IWindow>` で返す。そのため Result シーンは受け取った直後に `dynamic_cast<IResultWindowManager*>` で本来の型を取り出している。Select用の `createSelectWindowManager` は専用型（ISelectWindowManager）で返しており、同じファクトリ内で方針が非対称。戻り値型を IResultWindowManager にすれば dynamic_cast は消え、「返す型＝使う型」に揃う。

## AR-23.【低】UIManager / IUIElement 抽象が中途半端に導入されている

汎用UIフレームワークとして IUIElement（update/draw/setVisible）と UIManager（一括更新・描画）が用意されているが、IUIElement の実装は Button の1クラスだけ、UIManager の利用者は TitleView の1箇所だけである。一方 PauseMenuView は項目矩形の計算・ホバー判定・ラベル描画を独自実装しており、抽象の恩恵を受けていない。「全UIをIUIElementに乗せる」方向へ広げるか、逆に UIManager をTitleView専用ヘルパへ降格して汎用フレームワークの看板を下ろすか、どちらかに倒すべきである。中間の現状は、初見の読者に「UIはこの抽象に乗せるのがルールなのか？」という誤ったシグナルを送る。

## AR-24.【低・定数】PauseMenuViewのフォント名直書き

PauseMenuView は `"x12y16pxMaruMonica"` をローカル定数で直書きしている。Title / Lockscreen は `getFontName("main")` / `getFontName("normal")` でリソース設定（FontRepository）から取得しており、フォント差し替え時にポーズメニューだけが古いフォントのまま残る。ご指名の「定数化の仕組みがあるのに直書き」パターンのロジック版で、getFontNameへ寄せるだけで消える。

## AR-25.【低・定数】JSONアセットパスの散在と resources.json の二重読込

`"assets/config/resources.json"` のパスが ResourceManager と AudioRepository の2箇所に直書きされており、しかも AudioRepository は、兄弟リポジトリ4つ（Model/Font/Image/Animation）がコンストラクタでパース済みconfigを受け取る方式なのに対し、自分でファイルを開き直して再パースしている（方式の不一致＋無駄な二重I/O）。その他 `stageCatalog.json` `projectileData.json` `effectData.json` `stage-test.json` も各リポジトリ内の文字列直書きで、名前付き定数にしているのは ExtensionBonusRepository と StageRepository だけと流儀が割れている。`AssetPaths.h` のような集約ヘッダを作り、AudioRepository は他と同じconfig受け取りに統一するのが筋。

## AR-26.【低】シーンごとのフェードFSMの反復

Title / Lockscreen / Select / Loading はいずれも FadeIn / Idle / FadeOut 系のstate enumとFadeTransition生成・完了判定・遷移を個別に書いている。各シーンの状態集合が微妙に違うため強制的な基底クラス化はやり過ぎになり得るが、「フェード完了時にコールバックを呼ぶFadeSequence」程度の小さな部品を切るだけでも各シーンのswitch文が数行に縮む。優先度は低いが、シーンを今後追加するなら先に整えると追加コストが下がる。

## AR-27.【微小】コメントと値の不一致（ドキュメント腐敗の芽）

ChargeZoomSystem の `MAX_ZOOM_AMOUNT{ 0.1f }` に「1.0 - 0.2 = 0.8倍まで寄る」というコメントが付いており、値0.1（＝0.9倍）と食い違っている。調整で値だけ変えてコメントが置き去りになった形。このプロジェクトはコメントの質が高く「コメントを信じて読める」ことが強みなので、その信頼を守る意味で見つけ次第直す価値がある。

## AR-28.【確認事項・良い点の追記】

全数走査で確認できた良い設計も追記しておく。Component全33種はロジックを持たない純データであることを機械検査で確認した。core::log は最初から `#ifdef _DEBUG` でリリース時に消える設計になっており、デバッグ機構の`#ifdef`化（AR-6/パフォーマンスレポートPF-F）はログに関しては既に完了している。WindowConstants によるJSONキーの集約は MetadataKeys と同じ規律がplatform層にも通っている良例。EntityManagerの世代付きID、EventBusのpublish中解除への防御（穴埋め方式）、StagePropの純粋なビルダー構造、FileExtensionTypeResolver::fromPath の「手順を書き写させない」設計意図のコメントなど、細部の判断は一貫して質が高い。

---

# 総括と優先順位（改訂）

構造の骨格（レイヤー分離・DI・イベント駆動・ECS）は就活ポートフォリオとして胸を張れる水準にあり、違反は例外的である。だからこそ、数少ない違反が目立つ。直す順番としては、まず AR-1（platform内のステータス計算——game層に同じ計算が既にある二重実装と判明したため優先度がさらに上がった）と AR-3（勝利条件のラムダ埋没）、そして AR-19（死に方の敵種ハードコード——自ら宣言したデータ駆動方針との矛盾なので、面接で方針を語るなら先に潰しておくべき）。次に定数系（AR-12〜14、AR-24〜25）は合計1時間程度で消せる。AR-21（入力エッジ検出のシステムメンバ化）は今は動くが2体目で壊れる時限爆弾なので、安いうちに。AR-2（core/dataのドメイン型）と AR-5（ファットインターフェース）は工数が大きいので、方針だけ決めてコメントで意図を明文化しておき、時間があれば着手、でよい。AR-8（setter11本）とAR-20（徘徊FSMの共通化）は次にその周辺を触る機会に合わせるのが自然である。

---

# 第Ⅲ部

# リファクタリング実装手順書（最優先4項目＋定数一括修正）

作成日: 2026-07-25
対象: architecture_review.md の AR-1 / AR-3 / AR-19 / AR-21 と、定数系（AR-12〜14, 24〜25）の一括修正
方針: 実コードのシグネチャに合わせた具体案。ステップ順は依存が少ない順（＝途中で止めても壊れない順）に並べてある。

---

# STEP 1. 定数系の一括修正（所要 約1時間・リスクほぼゼロ）

先にこれを片付ける。ビルドが通ることだけ確認すればよい安全な変更で、以降のステップの下地（共有ヘッダの置き場の前例）にもなる。

## 1-1. 円周率（AR-12）

PlayerHUDView.cpp / ObjectiveView.cpp / HudPanel.cpp / InGameView.cpp の4ファイルで、無名namespace内の `constexpr float PI{ 3.141593f };` / `TWO_PI{ 6.283185f };` を削除し、`#include "core/utility/MathConstants.h"` を追加、参照を `core::utility::PI` / `core::utility::TWO_PI` に置換する。値がstd::numbers由来に変わるが、UIの円弧・軌道演出の見た目に知覚可能な差は出ない。

## 1-2. 基準解像度（AR-13）

core/constant/UI.h の `namespace core::constant::ui` に1行追加する。

```cpp
// UIレイアウトの基準解像度（高さ）。*_RATIO 系の定数はすべてこの高さを1.0とした比率
constexpr int BASE_SCREEN_HEIGHT{ 1080 };
```

PlayerHUDView / ObjectiveView / EquipmentSlotView / HudPanel の各cppからローカル定義を削除して参照を差し替える。既存の `FONT_SIZE_*_RATIO` のコメント（「28 / 1080」等）とも整合する置き場である。

## 1-3. 手動同期していた物理・演出定数（AR-14）

`game/constant/` に共有ヘッダを2つ新設する。

```cpp
// game/constant/DeathPhysics.h
#pragma once
namespace game::constant::death_physics
{
	// 死亡中の敵が地面で反発する係数。CollisionSystem（壁・箱）と
	// GroundingSystem（接地面）の両方で使うため、ここで一元管理する
	constexpr float BOUNCE_RESTITUTION{ 0.5f };
	// これより落下速度が遅くなったらバウンドをやめて静止させる
	constexpr float BOUNCE_MIN_SPEED{ 20.0f };
}
```

```cpp
// game/constant/TelegraphVisuals.h
#pragma once
namespace game::constant::telegraph
{
	// 予兆を地面からわずかに浮かせる量（Zファイティング防止）。
	// TelegraphVisualsSystem / AttackTelegraphVisualsSystem で共有する
	constexpr float GROUND_LIFT{ 2.0f };
}
```

CollisionSystem.cpp と GroundingSystem.cpp から `DEATH_BOUNCE_*` のローカル定義（と「CollisionSystemと揃える」コメント）を削除して差し替える。Telegraph系2ファイルも同様。`ANIM_FALLBACK_TIMEOUT{5.0f}`（EnemyDeath/PlayerDeath）は「死亡アニメの保険タイムアウト」なので DeathPhysics.h ではなく、両システムが読む場所として `game/constant/DeathSequence.h` を切るか、DeathPhysics.h を `DeathTuning.h` に改名して同居させる（推奨は後者：ファイルを増やしすぎない）。

## 1-4. フォント名（AR-24）

PauseMenuView の `MAIN_FONT_NAME{"x12y16pxMaruMonica"}` を削除する。PauseMenuController → PauseMenuView の生成経路はApplicationにあるので、Application のコンストラクタで `getFontName("main")` を引いて PauseMenuController のコンストラクタ引数に足し、Viewまで渡す。Title と同じ流儀になる。

## 1-5. アセットパス（AR-25）

`game` 層は触らず infrastructure 内で完結する。`infrastructure/resource/AssetPaths.h` を新設し、`resources.json` / `stage-test.json` / `stageCatalog.json` / `projectileData.json` / `effectData.json` / ボーナス設定のパスを集約。AudioRepository はコンストラクタで `const nlohmann::json&` を受け取る方式に変え（Model/Font/Image/Animationと同じ）、ResourceManager から渡す。これで resources.json の二重読込も同時に消える。

---

# STEP 2. AR-21: 入力エッジ検出をComponentへ移す（所要 約30分）

2体目の入力Entityで壊れる時限爆弾の解除。InputComponent に前回値と「押した瞬間」フラグを持たせ、計算は入力の責務元である InputSystem に一本化する。

## 2-1. InputComponent への追加

```cpp
// game/component/movement/InputComponent.h に追加
bool m_jumpJustPressed{ false };   // このフレームで押した瞬間（エッジ）
bool m_attackJustPressed{ false }; // 同上（攻撃）
bool m_prevJumpHeld{ false };      // 前フレームの押下状態（InputSystemだけが書く）
bool m_prevAttackHeld{ false };
```

## 2-2. InputSystem::update の末尾でエッジを確定

毎フレームの初期化ブロックで `m_jumpJustPressed = false; m_attackJustPressed = false;` を足し、キー・パッド読み取りがすべて終わった後に:

```cpp
input.m_jumpJustPressed   = input.m_jumpPressed   && !input.m_prevJumpHeld;
input.m_attackJustPressed = input.m_attackPressed && !input.m_prevAttackHeld;
input.m_prevJumpHeld   = input.m_jumpPressed;
input.m_prevAttackHeld = input.m_attackPressed;
```

## 2-3. 利用側の置き換え

PhysicsSystem から `m_prevJumpPressed` メンバと `jumpEdge` の計算を削除し、`const bool jumpEdge{ input.m_jumpJustPressed };` に置換。PlayerAttackComboSystem から `m_wasAttackPressed` を削除し、`isJustPressed` を `input.m_attackJustPressed` に置換。挙動は完全に等価で、状態がEntityに付いたぶんだけ正しくなる。

注意点が1つ: Application は固定タイムステップで update を0〜5回呼ぶが、入力のcapture/updatePreviousStateはフレームに1回である。InputSystem も update ごとに走るため、同一フレーム内の2回目以降の update では `m_prevJumpHeld` が既に更新済みで、エッジは1回しか立たない。これは「押した瞬間の判定はフレームに1回」という既存のApplication側の設計意図（コメントに明記あり）と一致するので問題ない。

---

# STEP 3. AR-19: 死に方をデータ駆動へ（所要 約1時間）

「差分はすべてJSON」という宣言を完全成立させる。

## 3-1. メタデータキーと EnemyData

```cpp
// game/constant/MetadataKeys.h に追加
constexpr std::string_view DEATH_STYLE{ "deathStyle" }; // "fall" | "animate"
```

EnemyData はfloat以外の値を持てるか確認が必要。floatProperties しか無い場合は ModelMetadata に stringProperties を足すより、専用フィールドが素直:

```cpp
// EnemyData に追加（fromMetadata内で読み取り）
enum class DeathStyle { Animate, Fall };
DeathStyle m_deathStyle{ DeathStyle::Animate };
// 読み取り: metadata.stringProperties か、JSONの "deathStyle" を直接。
// 無指定は Animate（現行のXcode/Macと同じ）にして既存JSONの後方互換を保つ
```

## 3-2. 死に方をComponentへ載せ替える

EnemyBase::buildCommonComponents（または DeathComponent 付与箇所）で、EnemyData の値を DeathComponent に書き込む:

```cpp
// game/component/combat/DeathComponent.h に追加
bool m_fallsOnDeath{ false };
```

## 3-3. EnemyDeathSystem の分岐を差し替え

```cpp
// isFallingDeath(componentManager, entityId) を削除し、呼び出し箇所を:
const auto& death{ m_componentManager.get<component::combat::DeathComponent>(entityId) };
if (death.m_fallsOnDeath) { /* 落下・バウンド死 */ }
```

これで `EnemyType` enum への依存が EnemyDeathSystem から消える。safariData.json に `"deathStyle": "fall"` を1行足せば挙動は現状維持。新しい浮遊敵はJSONだけで完結する。

---

# STEP 4. AR-1: セレクト画面のステータス計算をgame層へ（所要 2〜3時間）

platform→game 依存の根絶と二重実装の解消。鍵は「platformは計算結果のDTOを受け取って表示するだけ」にすること。

## 4-1. 共有DTOを core/data に置く

```cpp
// core/data/EquipmentPreview.h
#pragma once
namespace core::data
{
	/** @brief セレクト画面のパラメータ表示用スナップショット（計算はgame層が担う） */
	struct EquipmentPreviewStats
	{
		float m_baseHp{}, m_baseAtk{}, m_baseDef{}, m_baseSpd{};
		float m_bonusHp{}, m_bonusAtk{}, m_bonusDef{}, m_bonusSpd{};
		int   m_equippedSlots{};
	};
}
```

## 4-2. game層に計算サービスを新設（PlayerDataを再利用して二重実装を消す）

```cpp
// game/service/EquipmentPreviewService.h
#pragma once
#include <array>
#include <string>
#include "core/data/EquipmentPreview.h"
namespace core::iface { class IResourceManager; }

namespace game::service
{
	class EquipmentPreviewService
	{
	public:
		explicit EquipmentPreviewService(core::iface::IResourceManager& resourceManager);

		/** @brief 装備スロットのパス一覧から表示用ステータスを計算する */
		[[nodiscard]] core::data::EquipmentPreviewStats compute(
		    const std::array<std::string, 3>& slotPaths) const;
	private:
		core::iface::IResourceManager& m_resourceManager;
	};
}
```

```cpp
// game/service/EquipmentPreviewService.cpp（要点のみ）
core::data::EquipmentPreviewStats EquipmentPreviewService::compute(
    const std::array<std::string, 3>& slotPaths) const
{
	core::data::EquipmentPreviewStats out{};

	// 基礎値は PlayerData::fromMetadata を唯一の情報源にする
	// （metadata_keysの読み出しをここへ書き写さない＝二重実装の根絶）
	if (const auto meta{ m_resourceManager.getMetadata(constant::model_id::PLAYER) })
	{
		const auto base{ data::PlayerData::fromMetadata(*meta) };
		out.m_baseHp  = base.getMaxHp();
		out.m_baseAtk = base.getAttackPower();
		out.m_baseDef = base.getDefence();
		out.m_baseSpd = base.getMoveSpeed();
	}

	for (const auto& path : slotPaths)
	{
		if (path.empty()) continue;
		++out.m_equippedSlots;
		const auto type{ utility::FileExtensionTypeResolver::fromPath(path) };
		const auto& bonus{ m_resourceManager.getExtensionBonus(type) };
		out.m_bonusHp += bonus.hp;  out.m_bonusAtk += bonus.atk;
		out.m_bonusDef += bonus.def; out.m_bonusSpd += bonus.spd;
	}
	return out;
}
```

補足: 現行のインゲーム適用は `PlayerData::applyExtensionBonus`（加算）で、上のプレビューも同じ加算なので数値は一致する。将来計算式を変えるときは applyExtensionBonus とこの compute のどちらかに寄せて片方から呼ぶ形にすればよい（例: PlayerData に「基礎＋ボーナス一覧→最終値」の静的関数を持たせ、両者がそれを呼ぶ）。

## 4-3. platformへはコールバックで渡す

`IWindowFactory::createSelectWindowManager` の引数に計算コールバックを1本足す:

```cpp
virtual std::unique_ptr<ISelectWindowManager> createSelectWindowManager(
    std::function<void()> onGameStart,
    std::function<void(int, const std::string&)> onFileSlotChanged,
    std::function<core::data::EquipmentPreviewStats(const std::array<std::string, 3>&)> computePreview,
    IResourceManager& resourceManager) = 0;
```

SceneFactory（game層）で `EquipmentPreviewService` を生成し、`[svc](const auto& paths){ return svc->compute(paths); }` を渡す。Win32SelectWindowManager::updateParameterWindow は全計算を捨てて:

```cpp
void Win32SelectWindowManager::updateParameterWindow() noexcept
{
	if (!m_parameterWindow || !m_computePreview) return;
	const auto s{ m_computePreview(m_slotPaths) };
	m_parameterWindow->refresh(
	    s.m_baseHp, s.m_baseAtk, s.m_baseDef, s.m_baseSpd,
	    s.m_bonusHp, s.m_bonusAtk, s.m_bonusDef, s.m_bonusSpd,
	    s.m_equippedSlots);
}
```

## 4-4. 残りのgame依存を落とす

`m_slotExtTypes` と `game::utility::FileExtensionTypeResolver` の使用（型名表示用）は、拡張子タイプの解決もgame側の責務なので、`onFileSlotChanged` の戻り値（または追加コールバック）で解決済みタイプ名を受け取る形へ。`game/constant/ModelId.h`・`MetadataKeys.h` のincludeは4-3の時点で不要になり削除。FileSelectWindow.h の Resolver include も同様に落とす。完了判定は「`grep -rn '#include "game/' platform/` が0件」であること。

---

# STEP 5. AR-3: 勝敗・進行ルールを StageProgressionSystem へ（所要 2〜4時間）

## 5-1. イベントを1つ追加

```cpp
// game/event/InGameEvents.h に追加
struct GameOverEvent : public core::iface::IGameEvent
{
	bool m_isVictory{ false };
};
```

## 5-2. 新システム（InGameのラムダから状態と購読ごと移植）

```cpp
// game/system/StageProgressionSystem.h（要点）
class StageProgressionSystem : public core::ecs::ISystem
{
public:
	StageProgressionSystem(core::ecs::ComponentManager& cm, core::base::EventBus& bus,
	    factory::EnemySpawner& spawner, core::iface::IResourceManager& res,
	    std::unordered_set<core::ecs::EntityId> stageEnemyIds, core::ecs::EntityId playerId);
	void update(float deltaTime) override { m_elapsedTime += deltaTime; }

	// リザルト保存用の読み取り口
	[[nodiscard]] int   getKillCount() const noexcept { return m_killCount; }
	[[nodiscard]] float getElapsedTime() const noexcept { return m_elapsedTime; }
	[[nodiscard]] float getTotalDamageTaken() const noexcept { return m_totalDamageTaken; }
private:
	void onEnemyDead(const event::EnemyDeadEvent& e);      // killCount++、AI停止、全滅→spawnBoss()
	void onEnemyVanished(const event::EnemyVanishedEvent&);// Macなら publish(GameOverEvent{true})
	void onPlayerDeathFinished();                          // publish(GameOverEvent{false})
	void onAttackHit(const event::AttackHitEvent& e);      // 被ダメ集計
	void spawnBoss();                                      // InGame::spawnBossを移植
	// …メンバは InGame から m_killCount / m_stageEnemyIds / m_macId /
	// m_totalDamageTaken / m_elapsedTime を移動
};
```

## 5-3. InGame 側の残り

InGame の setupEvents は GameOverEvent の購読1本だけになる: リザルト保存（progression のゲッターから値を取る）→ カーソル表示 → Result へ遷移。敵撃破ログ・AI停止・ボス出現条件・勝利条件はすべて新システムへ移り、InGame は配線専任に戻る。移植時の注意は2点: (1) EnemyDeadEvent 内でやっていたAI停止（m_isActive=false）はゲーム進行というより死亡処理なので、EnemyDeathSystem 側へ移すのがより正確な置き場。 (2) BossAppearedEvent の publish は spawnBoss と一緒に新システムへ移動する。

## 5-4. 副産物

勝利条件が `StageProgressionSystem` という名前で読める・単体テストできるようになり、AR-4（シーン遷移のロケータ循環）の遷移箇所も GameOverEvent 経由で InGame の1箇所に集約されるため、将来 ISceneNavigator 化するときの変更点も1箇所で済むようになる。

---

# 実施順序のまとめ

STEP 1（定数）→ STEP 2（入力エッジ）→ STEP 3（死に方データ化）は互いに独立で、どれも半日以内・低リスク。STEP 4（AR-1）はインターフェース変更を含むため1コミットでまとめて通し、完了判定を「platformからgameのincludeが0件」に置く。STEP 5（AR-3）は挙動を1つずつ移すより「状態＋購読を丸ごと移植→InGameから削除→ビルド」の順が安全。全部やっても合計1.5〜2日の見積もりで、面接前に潰す価値のある範囲に収まる。