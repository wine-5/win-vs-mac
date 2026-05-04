## AttackSystem リファクタリング TODO

### ① `AttackComponent` の変更
- `m_isAutoAttack` / `m_isAttacking` を削除
- `m_attackRequested` フラグを追加

### ② `AIComponent` の変更
- `m_attackRange` を追加（攻撃範囲の判定に使用）
- `m_attackCooldown` / `m_currentAttackCooldown` を追加（1秒に1回攻撃フラグを立てる）

### ③ `AISystem` の変更
- 攻撃クールダウンを毎フレーム更新
- ターゲットが `m_attackRange` 内 かつ クールダウン = 0 のとき `AttackComponent::m_attackRequested = true` にセット

### ④ `InputComponent` の変更
- `m_attackPressed` フラグを追加

### ⑤ `InputSystem` の変更
- 左クリック（`isMouseLeftPressed`）で `m_attackPressed = true` にセット
- 毎フレームリセット処理を追加

### ⑥ `AttackSystem` の変更
- 距離計算を削除
- `m_attackRequested = true` のEntityだけ処理する
- InputComponentを持つEntityは `m_attackPressed` を確認してからCORを実行
- 処理後 `m_attackRequested = false` にリセット

### ⑦ `Enemy.cpp` の変更
- `AttackComponent` から `m_isAutoAttack` 関連の設定を削除

### ⑧ `Player.cpp` の変更
- `AttackComponent` に `m_attackRequested = false` のみ設定

---

## 開発が進んだときに行うべきこと

### 敵のモデルについて
- 現在の設計では同じモデルを複数生成する分にはメモリ負荷が敵に追加するComponentのみで済むが、別のモデルをたくさん生成するようになるとモデルだけでメモリ負荷がたくさんかかってしまう
- そのため敵のモデルが増えてきたときに、fbxをインポートしてゲームで描画できるような新規ライブラリを追加するべき

### リファクタリング
- 初期化の際は｛｝で初期化リストで初期化を行うべきだが、代入も{}にしてしまうと可読性が下がることが分かったため、
初期化は｛｝で、代入は = で明示的に書く様にする

- 現在DxLib.hを直接インクルードしているが、ラッパーのヘッダーファイルがあったほうが安全かつ差し替えるときに容易になるため作るようにする

- InGame.cpp内のhファイルが多いためhファイルをインクルードする専用クラスを作成するか検討中