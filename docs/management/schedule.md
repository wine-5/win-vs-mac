# 開発スケジュール

## 期間
2025年3月 〜 2025年10月（8ヶ月）

---

## フェーズ1：ECS基盤・最小構成（3月〜4月）

### 3月
- [x] フォルダ構成の作成
- [x] DxLibのセットアップ完了
- [x] `Entity`の実装
- [x] `ComponentManager`の実装
- [x] `SystemManager`の実装
- [x] `TransformComponent`の実装
- [x] `HealthComponent`の実装
- [x] `MoveSystem`の実装
- [x] `InputManager`の実装
- [x] プレイヤーがWASDで動く状態

### 4月
- [x] `SceneManager`の実装
- [x] `IScene`インターフェースの実装
- [x] `TitleScene`の実装
- [x] `GameScene`の仮実装（InGameSceneとして実装）
- [x] `Renderer`の実装（3D描画の基礎）
- [x] `Camera`のアイソメトリック実装
- [x] `AIComponent`の実装
- [x] `AISystem`の実装
- [x] 敵が動く状態

---

## フェーズ2：戦闘・Windows API連携（5月〜6月）

### 5月
- [ ] `WeaponComponent`の実装
- [ ] `BattleSystem`の実装
- [ ] 通常攻撃の実装
- [ ] ダッシュの実装
- [ ] `ISystemDataProvider`の実装
- [ ] `WindowsSystemProvider`の実装
- [ ] CPU使用率の取得・ゲームへの反映
- [ ] メモリ使用量の取得・ゲームへの反映

### 6月
- [ ] `IProcessProvider`の実装
- [ ] `WindowsProcessProvider`の実装
- [ ] プロセス列挙の実装
- [ ] レジストリ参照によるインストール済みアプリ検索
- [ ] 敵の種類をプロセスから生成
- [ ] `IFileSystemProvider`の実装
- [ ] `WindowsFileSystemProvider`の実装
- [ ] ファイル情報から武器・パラメータを生成

---

## フェーズ3：ダンジョン生成・全シーン実装（7月〜8月）

### 7月
- [ ] `DungeonManager`の実装
- [ ] `Room`の実装
- [ ] フォルダ構造からダンジョン自動生成
- [ ] 部屋間の移動実装
- [ ] `FileSelectScene`の実装
- [ ] エクスプローラー風UIの実装
- [ ] 難易度リアルタイム表示の実装
- [ ] `LoadingScene`の実装
- [ ] ロード演出の実装

### 8月
- [ ] `ProcessSystem`の実装
- [ ] ゲーム中のエクスプローラー呼び出し（スキル発動）
- [ ] 心拍数システムの実装
- [ ] ボス（`mac_os.exe`）の実装
- [ ] `ResultScene`の実装
- [ ] クリア画面の実装

---

## フェーズ4：仕上げ・就活準備（9月〜10月）

### 9月
- [ ] 全シーンの結合テスト
- [ ] バグ修正
- [ ] バランス調整
- [ ] サウンドの実装
- [ ] エフェクトの追加
- [ ] UIの仕上げ

### 10月
- [ ] 最終バグ修正
- [ ] README作成
- [ ] 設計ドキュメントの整備
- [ ] GitHubリポジトリの整理
- [ ] デモ動画の作成
- [ ] 就活用ポートフォリオへの追加

---

## 優先度について
以下の順で優先して実装する：

1. ゲームが動く状態を作る（フェーズ1・2）
2. Windows API連携を実装する（フェーズ2）
3. ダンジョン生成を実装する（フェーズ3）
4. 仕上げ（フェーズ4）

万が一スケジュールが遅れた場合は以下を削減候補とする：
- 心拍数システム
- サウンド
- エフェクト