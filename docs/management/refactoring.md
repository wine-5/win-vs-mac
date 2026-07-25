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