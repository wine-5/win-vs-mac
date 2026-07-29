#pragma once

namespace core::constant
{
	/**
	 * DEBUG: 開発用の切り替えをここへ集約する。
	 *
	 * 以前はソースの各所に「リリース時はfalseにすること」というコメント付きの
	 * 値が散らばっており、リリースビルドのたびに全部を探して直す必要があった。
	 * 切り替えたい値はこのファイルだけを見ればよいようにしている。
	 *
	 * 全層から見えるよう core層 に置いている（Main・Platform層はGameManagerより
	 * 先に動くため、GameManagerのフラグでは切り替えられない）。
	 */

	// Debug構成でビルドしているか。個別フラグの既定値として使う
#ifdef _DEBUG
	constexpr bool IS_DEBUG_BUILD{ true };
#else
	constexpr bool IS_DEBUG_BUILD{ false };
#endif

	// 調査用のログファイル（Log.txt / game_log.txt / memory_probe.txt）を書き出すか。
	// falseなら実行フォルダにこれらが作られない。
	// フリーズやメモリ使用量を追いたいときだけtrueに戻す
	constexpr bool WRITE_DEBUG_LOG_FILES{ false };

	// 起動時にデバッグ用シーン（破壊演出の検証）から始めるか。
	// falseなら通常どおりBIOSから始まる
	constexpr bool START_FROM_DEBUG_SCENE{ false };

	// デバッグ表示（当たり判定などのギズモ・統計HUD）を生成するか。
	// falseならインゲームで一切作られず、更新も描画も走らない
	constexpr bool ENABLE_DEBUG_VIEWS{ IS_DEBUG_BUILD };

	// 統計HUD（右上のFPS・CPU・メモリ）を最初から表示するか。
	// 右上は難易度・経過時間のHUDと重なるため、既定では出さない
	constexpr bool SHOW_DEBUG_HUD{ false };

	// インゲーム中にマウスカーソルを表示するか。
	// 本来は3人称マウス視点のため隠す。デバッグウィンドウを触りたいときだけtrue
	constexpr bool SHOW_MOUSE_CURSOR_IN_GAME{ false };

	// 連続ジャンプ（空中浮上）を許可するか。
	// falseで通常の接地単発ジャンプ。空中移動して動作確認したいときだけtrue
	constexpr bool ALLOW_CONTINUOUS_JUMP{ false };
} // namespace core::constant
