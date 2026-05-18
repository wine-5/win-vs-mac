# Web Layer 通信プロトコル

すべて JSON。`type` フィールドで種別を識別する。

---

## C++ ↔ JS 通信プロトコル

すべて JSON。`type` フィールドで種別を識別する。

### Job Window

| 方向 | type | フィールド | 説明 |
|---|---|---|---|
| JS → C++ | `jobSelected` | `job: string` | 職業選択（`"Warrior"` / `"Mage"` / `"Ninja"`） |
| C++ → JS | `refresh` | `job, hp, atk, def, spd, skill` | 選択職業のステータスを更新 |

```js
// JS → C++ 例
sendToGame({ type: 'jobSelected', job: 'Warrior' });

// C++ → JS 例
{ type: 'refresh', job: 'Warrior', hp: 120, atk: 100, def: 80, spd: 75, skill: '全方位斬り' }
```

### File Window

PC 上のファイルを 3 スロットに登録する。各スロットに紐付いたファイルの**拡張子グループ**（`FileExtensionType`）が
プレイヤーパラメータへのボーナスを決定する。

| 方向 | type | フィールド | 説明 |
|---|---|---|---|
| JS → C++ | `slotSelected` | `slot: number` | スロットをクリック → C++ がファイルダイアログを開く |
| C++ → JS | `refresh` | `slots: SlotInfo[]` | スロット一覧を更新（ファイル選択後） |

```ts
// SlotInfo 型
{
  slot: number,
  isEmpty: boolean,
  fileName: string | null,   // 表示用ファイル名（拡張子付き）
  extType: string | null,    // 'Executable' | 'Document' | 'Image' | 'Audio' | 'Archive' | 'Unknown'
  bonusDesc: string | null   // 例: '+10 ATK' 、'+20 HP' など（C++ 側で文字列化）
}
```

### Parameter Window

ジョブ基本値（`base`）とファイル拡張子ボーナス（`bonus`）を分けて受け取り、スタックバーで表示する。

| 方向 | type | フィールド | 説明 |
|---|---|---|---|
| C++ → JS | `refresh` | 下記参照 | ステータス全更新 |

```js
// C++ → JS 例
{
  type: 'refresh',
  job: '魔法使い',
  skill: '巨大魔法弾',
  slot: 1,
  // base = jobData.json の値（ジョブ固有）
  baseHp: 80,  baseAtk: 60,  baseDef: 20,  baseSpd: 40,
  // bonus = ExtensionBonusCalculator の出力（ファイル拡張子由来）
  bonusHp: 20, bonusAtk: 0,  bonusDef: 0,  bonusSpd: 0
}
```

### Difficulty Window（実装済み）

| 方向 | type | フィールド | 説明 |
|---|---|---|---|
| JS → C++ | `difficultySelected` | `difficulty: string` | `"Easy"` / `"Normal"` / `"Hard"` |

---

## 更新フロー

```
職業選択（job.js）
  └─ sendToGame({ type: 'jobSelected', job }) ──→ C++ (SelectScene)
       └─ postMessage({ type: 'refresh', ... }) ─→ param.js（base + bonus で再描画）
                                                 ─→ job.js（ハイライト同期）

ファイル選択（file.js）
  └─ sendToGame({ type: 'slotSelected', slot }) ─→ C++ (SelectScene)
       └─ ファイルダイアログを開く → 拡張子を判定 → bonus を計算
            └─ postMessage({ type: 'refresh', ... }) ─→ param.js（bonus 更新）
                                                      ─→ file.js（スロット表示更新）
```

C++ 側がステータス計算後、Job / File / Param の各ウィンドウに `refresh` を投げる。

