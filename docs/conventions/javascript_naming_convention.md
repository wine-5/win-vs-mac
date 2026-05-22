# JavaScript / Web UI 命名規則

このドキュメントは、Web UI（JavaScript・HTML・CSS）における命名規則を定義します。
**ベース：[Google JavaScript Style Guide](https://google.github.io/styleguide/jsguide.html)**

C++側の命名規則は [naming_convention.md](naming_convention.md) を参照してください。

---

## ファイル・フォルダ構成

| 対象 | 規則 | 例 |
|---|---|---|
| JavaScriptファイル | ケバブケース | `job-logic.js`, `job-view.js`, `result-view.js` |
| HTMLファイル | ケバブケース | `job.html`, `result.html` |
| CSSファイル | ケバブケース | `job.css`, `result.css` |
| フォルダ名 | 小文字 | `select/`, `result/`, `common/` |

**参考：** Google Style Guide では filename.js 形式（ハイフンは許可）

---

## JavaScript（コード内の命名）

### IIFEパターン（モジュール）のクラス名

IIFE（Immediately Invoked Function Expression）で定義するモジュール名は **PascalCase**：

```javascript
const JobLogic = (function () {
    // ...
    return { /* 公開メソッド */ };
}());

const JobView = (function () {
    // ...
    return { /* 公開メソッド */ };
}());
```

**Google Style Guide:** コンストラクタ・クラスは CapWords (PascalCase)

### 関数名

すべての関数は **camelCase**：

```javascript
function initialize() { }
function renderSlots() { }
function updateSelection(jobId) { }
function calculateBar(baseVal, bonusVal) { }
function getDescription(difficulty) { }
```

**Google Style Guide:** 関数は camelCase

### 変数名（ローカル・メンバ）

ローカル変数・メンバ変数は **camelCase**：

```javascript
let jobListElement = null;      // DOM要素
let currentHighlight = null;    // 状態変数
let selectedSlot = 0;           // プリミティブ値
const jobStats = getJobStats(); // ローカル定数
```

#### DOM要素の変数名ルール

DOM要素を保持する変数には **`Element` または `El` サフィックス** を付与して可読性を向上：

```javascript
let btnEasyElement = null;      // ✅ 推奨：Element サフィックス
let descEl = null;              // ✅ OK：El サフィックス
let fileListElement = null;     // ✅ 推奨
let statusEl = null;            // ✅ OK
```

**Google Style Guide:** 変数は camelCase。DOM要素は Element/El サフィックスで明示化することを推奨

### 定数

**グローバル・モジュール スコープの定数** は **CONSTANT_CASE**：

```javascript
const MAX_SLOTS = 3;
const DEFAULT_DIFFICULTY = 'NORMAL';
const EXT_ICON = { /* ... */ };  // オブジェクト定数も大文字
const DIFFICULTY_DESCRIPTIONS = { /* ... */ };
```

**Google Style Guide:** `@const` で注釈された値、または const で宣言で再割り当て不可な値は CONSTANT_CASE

**ローカル定数** は camelCase でOK：

```javascript
const jobStats = fetchStats();  // 関数呼び出し結果
const baseValue = 100;          // リテラル値
```

---

## HTML・DOM

### IDの命名

HTML要素の `id` は **ケバブケース** (lowercase-with-hyphens)：

```html
<div id="job-list"></div>
<input id="btn-easy">
<span id="diff-desc"></span>
<div id="result-root"></div>
<div id="bsod-overlay"></div>
```

**Google Style Guide:** IDとクラスは lowercase-with-hyphens

### data-属性の命名

`data-*` 属性は **ケバブケース**：

```html
<div data-id="job-1"></div>
<button data-action="retry"></button>
<div data-slot="0"></div>
<div data-ext="ts"></div>
```

**参考：** HTML標準でも data-* は lowercase-with-hyphens が推奨

---

## CSS

### クラス名

すべてのCSSクラスは **ケバブケース** (lowercase-with-hyphens)：

```css
.job-card { }
.job-icon { }
.job-name { }
.active-easy { }
.result-file-badge { }
.metric-row { }
.file-icon { }
.bar-flash { }
```

**Google Style Guide:** クラスセレクタは lowercase-with-hyphens

### BEM (Block Element Modifier) パターン

より大規模なコンポーネントではBEM記法の使用を検討：

```css
/* Block（コンポーネント全体） */
.job-card { }

/* Element（ブロック内の要素） */
.job-card__header { }
.job-card__icon { }
.job-card__stats { }

/* Modifier（状態変化） */
.job-card--active { }
.job-card--disabled { }
```

**既存コードベースでの許容：** 簡潔さのため、ハイフンによる単純な接続も許容：

```css
.job-card { }
.job-header { }  /* .job-card 内の要素 */
.job-icon { }
```

---

## イベント・コールバック

### イベントハンドラ関数名

コールバック登録メソッドは **camelCase** で `on` + 動作を表現：

```javascript
JobLogic.onJobSelected(function (jobId) { });
JobLogic.onStatsUpdate(function () { });
FileLogic.onSlotChange(function () { });
DifficultyLogic.onDifficultyChange(function (difficulty) { });
```

**Google Style Guide:** コールバックは camelCase。on で始まる命名は標準的

### メッセージハンドラ

`window.onMessageFromGame` は固定名（C++側との通信規約）：

```javascript
window.onMessageFromGame = function (data) {
    JobLogic.onMessageFromGame(data);
};
```

---

## 状態管理・データ構造

### 状態オブジェクト

状態を持つオブジェクトのプロパティは **camelCase**：

```javascript
const state = {
    baseHp: 100,
    bonusHp: 10,
    baseAtk: 50,
    currentJob: 'Warrior',
    totalDamageTaken: 25
};
```

**Google Style Guide:** オブジェクトプロパティは camelCase

### 列挙値・状態定数

ゲーム状態を表す文字列定数は **UPPER_SNAKE_CASE**（または既存の UPPER_CASE）：

```javascript
// ✅ 推奨：明示的な定数
const DIFFICULTY_EASY = 'EASY';
const DIFFICULTY_NORMAL = 'NORMAL';
const DIFFICULTY_HARD = 'HARD';

// OK：既存コードの慣習に従う
if (difficulty === 'EASY') { }
if (difficulty === 'NORMAL') { }
if (difficulty === 'HARD') { }
```

**参考：** 既存コードベースとの統一性を優先

---

## URL・パス

### 画像パス

画像パスは絶対URLを優先。WebView環境では `https://assets.game.web/` ドメインを使用：

```javascript
// ✅ 推奨：絶対URL（WebView環境）
'https://assets.game.web/images/ui/select/job-warrior.png'

// OK：相対パス（フォールバック）
'../../../assets/images/ui/select/job-warrior.png'
```

**参考：** WebViewの基URL (`https://game.web/`) とアセットドメイン (`https://assets.game.web/`) の区別を意識

---

## コメント・ドキュメント

JavaScriptのコメント規則は C++ の Doxygen ルールと異なります。

### 関数のドキュメントコメント

複雑な関数にはJSDoc形式のコメントを推奨（ただし必須ではない）：

```javascript
/**
 * 難易度を変更し、必要に応じて確認ダイアログを表示する
 * @param {string} difficulty - 難易度（'EASY', 'NORMAL', 'HARD'）
 */
function selectDifficulty(difficulty) {
    // ...
}
```

**Google Style Guide:** JSDoc は public API に推奨（内部関数は不要）

### インライン説明

WHY が非明白な場合のみコメント記載（C++と同じ）：

```javascript
// プログレスバーを再計算するため offsetWidth にアクセス
void el.offsetWidth;
```

---

## その他のルール

### グローバル変数

グローバル変数の使用は最小限に。モジュール（IIFE）内のローカル変数を優先：

```javascript
// ❌ 避ける
let globalJobData = null;

// ✅ 推奨
const JobLogic = (function () {
    let jobData = null;  // IIFE内に閉じ込める
    return { /* 公開メソッド */ };
}());
```

**Google Style Guide:** グローバルスコープの污染を避ける

### null / undefined の初期値

変数初期化は明示的に `null` で統一：

```javascript
let buttonElement = null;   // ✅ 明示的
let jobListElement = null;  // ✅ 明示的
let firstRender = true;     // ✅ 初期値が決まっている場合は値で
```

### 文字列リテラル

日本語文字列は UTF-8 で記載。メッセージ定数化は任意：

```javascript
statusEl.textContent = 'ファイルをクリックして選択';  // ✅ 直接記載OK
```

---

## まとめ：命名規則クイックリファレンス

| 対象 | 規則 | 例 |
|---|---|---|
| ファイル名 | ケバブケース | `job-logic.js` |
| フォルダ名 | 小文字 | `select/` |
| IIFE モジュール名 | PascalCase | `JobLogic`, `JobView` |
| 関数名 | camelCase | `initialize()`, `renderSlots()` |
| ローカル変数 | camelCase | `jobCount`, `userName` |
| DOM要素変数 | camelCase + Element/El | `jobListElement`, `btnEl` |
| グローバル定数 | CONSTANT_CASE | `MAX_SLOTS`, `DEFAULT_DIFFICULTY` |
| HTML id | ケバブケース | `job-list`, `btn-easy` |
| data-属性 | ケバブケース | `data-action`, `data-slot` |
| CSSクラス | ケバブケース | `.job-card`, `.active-easy` |
| コールバック関数 | on + camelCase | `onJobSelected()`, `onStatsUpdate()` |
| オブジェクトプロパティ | camelCase | `baseHp`, `currentJob` |

---

## 参考資料

- **[Google JavaScript Style Guide](https://google.github.io/styleguide/jsguide.html)** - 公式スタイルガイド
- **[MDN JavaScript Guide](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide)** - Mozilla公式ドキュメント
- **[ES6 標準](https://www.ecma-international.org/ecma-262/)** - ECMAScript仕様
