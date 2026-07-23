# フォルダ構成リファクタリング方針

- 評価日: 2026-07-23
- 対象: `src/` 一式
- 目的: 1ディレクトリあたりのファイル数が膨らんだ箇所を、責務ごとのサブフォルダへ分割する
- 関連: [refactoring.md](refactoring.md)（コード側のリファクタリング項目）

---

## 現状のファイル数（多い順）

| ディレクトリ | ファイル数 | クラス数 | 判定 |
|---|---|---|---|
| `src/game/system/` | 46 | 23 System | **要分割（最優先）** |
| `src/game/scene/` | 26 | 13 | 分割不要 |
| `src/game/component/` | 22 | 22 | 要分割 |
| `src/infrastructure/` | 20 | 10 | 要分割 |
| `src/core/interface/` | 17 | 17 | 検討レベル |
| `src/infrastructure/repository/` | 16 | 8 | 分割不要 |
| `src/game/factory/` | 14 | 7 | 分割不要 |
| `src/platform/window/select/` | 12 | 6 | 分割不要 |

---

## 1. `game/system` の分割（優先度: 高）

### ①現状

23個の System のうち、AI系5個だけが `ai/` サブフォルダにあり、残り18個が直下にフラットに並んでいる。
特に「〜VisualsSystem」「〜EffectSystem」という命名の演出系が8個溜まっており、一覧性が失われている。

### ②深刻度

**高**。System は今後も増える箇所であり、放置すると増え続ける。

### ③改善案

責務で4つのサブフォルダへ分割する。

```
system/
├── ai/          （既存のまま）
│   DetectionSystem, EnemyRangedAttackSystem, MacAISystem,
│   MeleeChaseAISystem, RangeKeepAISystem
│
├── combat/
│   AttackSystem, CollisionSystem, TargetingSystem,
│   PlayerRangedAttackSystem, EnemyDeathSystem,
│   ProjectileSystem, ProjectileReflectSystem, ProjectileWindowSystem
│
├── movement/
│   InputSystem, MoveSystem, PhysicsSystem
│
├── camera/
│   CameraSystem, DebugCameraSystem, ChargeZoomSystem, DamageShakeSystem
│
└── visual/
    AnimationSystem, EffectSystem, HitEffectSystem,
    TelegraphVisualsSystem, AttackTelegraphVisualsSystem,
    DetectionAlertVisualsSystem, PlayerChargeVisualsSystem,
    MacAwakenEffectSystem
```

### ④オーバーエンジニアリング判断

**やるべき**。ただし `combat/` の中をさらに `projectile/` へ切るのは**やらない**。
弾関連は3個しかなく、1〜3ファイルのフォルダが乱立すると逆に読みにくくなる。

`ChargeZoomSystem` / `DamageShakeSystem` はカメラを揺らす／寄せる演出であり、
`visual/` ではなく `camera/` に置く（操作対象がカメラのため）。

---

## 2. `game/component` の分割（優先度: 中）

### ①現状

`ai/` サブフォルダのみ存在し、残り18個の Component が直下にフラット。

### ②深刻度

**中**。全て1ファイル1ヘッダで中身が小さいため、system ほど切迫していない。

### ③改善案

System と同じ軸で揃え、System ↔ Component の対応関係を見えるようにする。

```
component/
├── ai/        （既存）
│   MacAIComponent, MeleeChaseAIComponent, PatrolComponent, RangeKeepAIComponent
│
├── combat/
│   AttackComponent, ColliderComponent, HealthComponent, ProjectileComponent,
│   DeathComponent, AimComponent, PlayerChargeComponent, TelegraphComponent
│
├── visual/
│   RenderComponent, AnimationComponent, AnimationClip,
│   EffectComponent, HitEffectComponent, CameraEffectComponent
│
└── core/
    TransformComponent, VelocityComponent, TagComponent, InputComponent,
    CameraComponent, AlertComponent, EnemyTypeComponent, AIComponent
```

### ④オーバーエンジニアリング判断

**System の分割とセットでやる**。単独で先行させる価値は薄い。
System を分けた後に同じ軸を適用するのが自然。

---

## 3. `infrastructure` の分割（優先度: 中）

### ①現状

`repository/` と `utility/` はサブフォルダ化されているのに、本体の10クラスが直下にフラットに並んでいる。

### ②深刻度

**中**。10クラスは限界ではないが、分割の軸が明確なので実施コストが低い。

### ③改善案

```
infrastructure/
├── graphics/   Renderer, UIRenderer, Camera, Screen, Animator
├── effect/     EffectFactory, EffectPool
├── audio/      AudioManager
├── input/      InputManager
├── resource/   ResourceManager
│   └── repository/   （既存の8 Repository をこの下へ移動）
└── utility/    （既存）
```

`repository/` を `resource/` 配下に入れることで「リソース読み込み一式」が1箇所にまとまる。

### ④オーバーエンジニアリング判断

**やるべき**。`audio/` `input/` が1クラスずつになるのは許容する。
これは実装が増える余地のある区分であり、かつ `core/interface` 側の分割軸と一致するため。

---

## 4. `core/interface` の分割（優先度: 低／要検討）

### ①現状

17個のインターフェースヘッダがフラット。

### ②深刻度

**低**。全て1行の抽象定義で中身が薄く、`I` プレフィックスでソートされているため一覧性は保てている。

### ③改善案（実施する場合）

infrastructure の分割軸と揃えることが実施条件。

```
interface/
│   ILogger, IStringConverter          （汎用）
├── graphics/   IRenderer, IUIRenderer, ICamera, IScreen, IAnimator
├── resource/   IResourceManager, IEffectFactory
├── platform/   IWindow, IWindowFactory, ISelectWindowManager,
│               IResultWindowManager, IProjectileWindowManager
└── system/     IInputProvider, IAudioManager, IPerformanceDataProvider
```

### ④オーバーエンジニアリング判断

**単独ではやらない**。infrastructure（項目3）を分割した後、軸を揃える目的でのみ実施する。
infrastructure を分割しないなら、ここも現状維持でよい。

---

## 5. 分割しないと判断したもの

| ディレクトリ | 理由 |
|---|---|
| `game/scene/`（26） | `Xxx.h/.cpp` + `XxxView.h/.cpp` のペア構成で、シーン名でソートされ既に読みやすい。シーンごとにフォルダを切ると1〜2ファイルのフォルダが乱立して逆効果。`SceneManager` / `SceneFactory` / `IScene` / `SceneType` の基盤4つも直下のままでよい |
| `infrastructure/repository/`（16） | 8つの Repository が完全に同格で、分ける軸が存在しない |
| `game/factory/`（14） | 同上。7クラスは許容範囲 |
| `platform/window/select/`（12） | Win32ウィンドウ5種＋Manager。これ以上割れない |
| `game/constant/`（8） | 定数定義のみ。フラットで問題なし |
| `core/ecs/`（8） | ECS基盤の中核。分けると逆に追いにくい |

---

## 実施タイミング

**[refactoring.md](refactoring.md) の推奨着手順（デッドコード削除 → EntityId世代化 → …）を一通り終えた後に実施する。**

理由: フォルダ移動は include パスの変更差分を大量に生むため、先に行うと本質的なコード変更が差分に埋もれる。
また、デッドコード削除でファイルが減る（`SelectView` / `Label` 等）ため、削除後の構成で分割軸を決めるほうが正確。

### コミット単位

1コミット＝1ディレクトリの分割とし、include パス修正まで含めて完結させる。

```
refactor: Systemを責務ごとのサブフォルダへ分割
refactor: Componentを責務ごとのサブフォルダへ分割
refactor: infrastructureを機能ごとのサブフォルダへ分割
refactor: core/interfaceをinfrastructureの構成に合わせて分割
```
