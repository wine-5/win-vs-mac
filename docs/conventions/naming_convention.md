# 命名規則

## ファイル名
- すべて `PascalCase`
  - 例: `Player.cpp`, `EnemyManager.h`, `Main.cpp`

## フォルダ名
- すべて小文字
  - 例: `src/`, `game/`, `enemy/`

## クラス名
- `PascalCase`
  - 例: `Player`, `EnemyManager`, `SceneManager`

## インターフェース名
- `I` + `PascalCase`
  - 例: `ISystemDataProvider`, `IEnemy`

## 関数名
- `camelCase`
  - 例: `getCpuUsage()`, `updatePlayer()`

## 変数名
- ローカル変数: `camelCase`
  - 例: `cpuUsage`, `playerPosition`
- メンバ変数: `m_` + `camelCase`
  - 例: `m_cpuUsage`, `m_playerPosition`
- 静的メンバ変数: `s_` + `camelCase`
  - 例: `s_instance`
- 定数 / constexpr: `UPPER_SNAKE_CASE`
  - 例: `MAX_ENEMY_COUNT`, `SCREEN_WIDTH`

## 列挙型
- 型名: `PascalCase`
- 値: `PascalCase`
  - 例:
```cpp
    enum class EnemyState
    {
        Idle,
        Chase,
        Attack
    };
```

## 名前空間
- すべて小文字
- 階層は `::` で表現する
  - 例: `namespace game`, `namespace game::player`, `namespace game::enemy`

## マクロ
- 基本的には使用禁止
- 定数は `constexpr`、条件分岐は `if constexpr`、インクルードガードは `#pragma once` を使用する

## コメント規則

### Doxygenコメント
**すべてのpublic関数・メソッドには必ずDoxygenコメントを記載する。**
Visual Studioでホバー時にガイドが表示されるようにし、他者が見ても理解しやすくするため。

#### 記載必須項目
- `@brief` : 関数の簡潔な1行説明（必須）
- `@param` : パラメータの説明（引数がある場合）
- `@return` : 戻り値の説明（void以外の場合）

#### フォーマット例
```cpp
/**
 * @brief Entityの生成・破棄を管理するクラス
 */
class EntityManager
{
public:
    /**
     * @brief Entityを生成する
     * @return 生成したEntity
     */
    Entity create();

    /**
     * @brief Entityを破棄する
     * @param entity 破棄するEntity
     */
    void destroy(Entity entity);

    /**
     * @brief 指定したIDのEntityを取得する
     * @param id 取得したいEntityのID
     * @return Entityのインスタンス
     * @throws std::out_of_range IDが存在しない場合
     */
    Entity getEntity(EntityId id) const;
};
```

#### 1行コメント形式（簡易版）
シンプルなgetter/setterの場合は `///` を使った簡易形式も可：
```cpp
/// Entityの総数を取得
size_t getEntityCount() const noexcept;

/// Entityが有効かどうかを判定
bool isValid(EntityId id) const noexcept;
```

### 通常コメント
処理の説明や内部のコメントは通常コメントを使う。
```cpp
// 再利用可能なEntityIdのキュー
std::queue<EntityId> m_recycledIds;

// 初期化処理
void initialize()
{
    // リソースを読み込む
    loadResources();
    
    // システムをセットアップ
    setupSystems();
}
```

### privateメソッドのコメント
privateメソッドについては、複雑な処理の場合のみコメントを記載（任意）。
```cpp
private:
    /**
     * @brief リソースファイルを読み込む
     * @details JSONファイルをパースしてメタデータを生成する
     */
    void loadResources();
```

---

## メンバ変数の初期化規則

**Uniform Initialization（統一初期化）** を使用し、未初期化変数によるバグを防ぐ。

メンバ変数は**クラス定義時に初期化する**ことを推奨。

| 型 | 初期化方法 | 例 | 理由 |
|---|---|---|---|
| **プリミティブ型** | `{}` | `int m_count{};`<br>`float m_speed{};`<br>`bool m_flag{};` | ゴミ値防止（0/false/nullptrで初期化） |
| **ポインタ** | `{}` | `Player* m_ptr{};` | nullptr明示 |
| **スマートポインタ** | `{}` | `std::unique_ptr<Player> m_player{};` | nullptr明示 |
| **enum型** | `= 値` | `CollisionTag m_tag = CollisionTag::None;` | デフォルト値を明示 |
| **STLコンテナ** | 初期化なし | `std::vector<int> m_data;`<br>`std::string m_name;` | デフォルトコンストラクタで十分 |
| **クラス型** | 初期化なし | `Vector3 m_pos;` | デフォルトコンストラクタに任せる |
| **クラス型（初期値重要）** | `{}` 明示 | `Vector3 m_scale{1.0f, 1.0f, 1.0f};` | 明示的初期化 |
| **参照型** | 初期化リスト | `RenderSystem(IRenderer& r) : m_renderer(r) {}` | 参照は定義時初期化不可 |
| **constexpr定数** | `= 値` | `static constexpr float MAX_SPEED = 10.0f;` | 定数 |

### Uniform Initialization（統一初期化）とは
C++11で導入された `{}` による初期化方法。正式には**List Initialization**。

**メリット:**
- 縮小変換を禁止（型安全）: `int x{3.14};` はコンパイルエラー
- すべての型で統一的に使える
- 未初期化変数を防ぐ: `int x{};` は必ず0になる

**ローカル変数の初期化**
```cpp
void someFunction()
{
    int count{};              // 0で初期化（推奨）
    float distance{5.0f};     // 値を指定
    auto ptr = std::make_unique<Player>();  // スマートポインタはmake系を使用
}
```

### 注意点
- 参照型とconst型は**必ずコンストラクタ初期化リストで初期化**（定義時初期化は不可）
- `{}`は値を0に初期化する（`int x{};` → 0、`bool flag{};` → false、`ptr{};` → nullptr）
- 初期化の順序は**宣言順**であり、初期化リストの記述順ではない
- 関数内のローカル変数も同様のルールを適用

---