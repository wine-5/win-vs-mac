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