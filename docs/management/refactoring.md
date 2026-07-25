# Win vs Mac パフォーマンス検査レポート v2

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