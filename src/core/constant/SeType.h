#pragma once
#include <string_view>
#include <utility>

namespace core::constant
{
    /**
     * @brief SE（効果音）の種類を表す列挙型
     */
    enum class SeType
    {
        None,

		// ---- プレイヤーの攻撃 ----
		PlayerSwing1,    // 近接コンボ1段目の振り
		PlayerSwing2,    // 近接コンボ2段目の振り
		PlayerCharge,    // 溜め撃ちの溜め開始
		PlayerChargeReady, // 溜め撃ちが最大まで溜まった（ここから先は溜めても無駄という合図）
		PlayerChargeRelease, // 溜めを解いて撃った
		HitWindow,       // Window弾が命中した
		HitChargedWindow, // 溜め撃ちのWindow弾が命中した
		Critical,         // クリティカルの一撃が出た

		// ---- 被弾・撃破 ----
		HitEnemy,        // 敵がダメージを受けた
		HitPlayer,       // プレイヤーがダメージを受けた
        DeadEnemy,       // 敵が倒れた
		DeadPlayer,      // プレイヤーが倒れた

		// ---- プレイヤーの挙動 ----
		PlayerJump,     // ジャンプした
		PlayerFootstep, // 一歩ぶんの足音

		// ---- 敵 ----
		EnemyAlert, // 敵がプレイヤーを発見した
		EnemySlam,  // Xcodeの地面叩きつけ

		// ---- 壊せるブロック ----
		BlockHit,         // ブロックを殴ったがまだ壊れない
		BlockHitCritical, // 次の一撃で壊れる打撃（あと1回を音で知らせる）
		BlockBreak,       // ブロックが砕けた

		// ---- 拡張子の欠片 ----
		ItemDrop,   // 欠片が出現した
		ItemPickup, // 欠片を拾った

		// ---- 進行・演出 ----
		BattleReady, // 開始演出のREADY
		BattleFight, // 開始演出のFIGHT!
		HudRattle,   // ボス覚醒でHUDが震え出した
		HudDrop,     // 震えたHUDが落ちて画面下端に当たった

		// ---- 拡張子の付け替え ----
		ExtensionGrab, // インベントリでマスを掴んだ
		ExtensionDrop, // 掴んでいたものを置いた・取り消した
		ExtensionSwap, // 装備中と未装備を入れ替えた
		ExtensionRejected, // 動かせない枠へ落とそうとして弾かれた

		// ---- UI ----
		UiClick,      // ボタンなどを押した
		UiKeyPress,   // 選択の移動・キー入力
		UiFileSelect, // 装備するファイルを決定した
		InventoryOpen, // インベントリを開いた
		PauseOpen,     // ポーズメニューを開いた
		UiClose,       // 画面を閉じた（インベントリ・ポーズで共用）

		// 今後追加したら SE_TYPE_NAMES にも追加を忘れないように
	};

	/**
	 * @brief JSONに書くSE名と列挙の対応表
	 *
	 * resources.json の音源定義と、敵の定義JSON（叩きつけ音など）の両方がこの名前を使う。
	 * 対応表を1つに保つことで、書ける名前がファイルごとにずれないようにする。
	 *
	 * 要素数は書かずコンパイラに数えさせる。数を書くと種別を足すたびにそこも直す
	 * 必要があり、忘れるとビルドが通らなくなるだけで何の得も無い
	 */
	inline constexpr std::pair<std::string_view, SeType> SE_TYPE_NAMES[]{
		{ "PlayerSwing1", SeType::PlayerSwing1 },
		{ "PlayerSwing2", SeType::PlayerSwing2 },
		{ "PlayerCharge", SeType::PlayerCharge },
		{ "PlayerChargeReady", SeType::PlayerChargeReady },
		{ "PlayerChargeRelease", SeType::PlayerChargeRelease },
		{ "HitWindow", SeType::HitWindow },
		{ "HitChargedWindow", SeType::HitChargedWindow },
		{ "Critical", SeType::Critical },
		{ "HitEnemy", SeType::HitEnemy },
		{ "HitPlayer", SeType::HitPlayer },
		{ "DeadEnemy", SeType::DeadEnemy },
		{ "DeadPlayer", SeType::DeadPlayer },
		{ "PlayerJump", SeType::PlayerJump },
		{ "PlayerFootstep", SeType::PlayerFootstep },
		{ "EnemyAlert", SeType::EnemyAlert },
		{ "EnemySlam", SeType::EnemySlam },
		{ "BlockHit", SeType::BlockHit },
		{ "BlockHitCritical", SeType::BlockHitCritical },
		{ "BlockBreak", SeType::BlockBreak },
		{ "ItemDrop", SeType::ItemDrop },
		{ "ItemPickup", SeType::ItemPickup },
		{ "BattleReady", SeType::BattleReady },
		{ "BattleFight", SeType::BattleFight },
		{ "HudRattle", SeType::HudRattle },
		{ "HudDrop", SeType::HudDrop },
		{ "UiClick", SeType::UiClick },
		{ "UiKeyPress", SeType::UiKeyPress },
		{ "UiFileSelect", SeType::UiFileSelect },
		{ "ExtensionGrab", SeType::ExtensionGrab },
		{ "ExtensionDrop", SeType::ExtensionDrop },
		{ "ExtensionSwap", SeType::ExtensionSwap },
		{ "ExtensionRejected", SeType::ExtensionRejected },
		{ "InventoryOpen", SeType::InventoryOpen },
		{ "PauseOpen", SeType::PauseOpen },
		{ "UiClose", SeType::UiClose },
	};

	/**
	 * @brief JSONに書かれたSE名を列挙へ変換する
	 * @param name SE名（SE_TYPE_NAMES のいずれか）
	 * @return 対応する種別。知らない名前ならNone（＝無音）
	 */
	[[nodiscard]] constexpr SeType toSeType(std::string_view name) noexcept
	{
		for (const auto& [key, type] : SE_TYPE_NAMES)
		{
			if (key == name)
				return type;
		}
		return SeType::None;
	}
} // namespace core::constant
